// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include <catch2/catch_test_macros.hpp>

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QTimeZone>

#include <misc/pathHelpers.h>
#include <storage/appSettingsStore.h>

using namespace kemai;

// No test sets autoLogin on save: that would write the token to the real keychain.
namespace
{
    QJsonObject makeSettingsJson(const QString &version)
    {
        QJsonObject json{
            {"settings", QJsonObject{{"autoLogin", QJsonObject{{"host", "https://kimai.example"}}}}}
        };
        if (!version.isNull())
        {
            json.insert("version", version);
        }
        return json;
    }

    QByteArray readFile(const QString &path)
    {
        QFile file(path);
        REQUIRE(file.open(QIODevice::ReadOnly));
        return file.readAll();
    }

    void writeFile(const QString &path, const QByteArray &content)
    {
        QFile file(path);
        REQUIRE(file.open(QIODevice::WriteOnly));
        file.write(content);
    }
} // namespace

TEST_CASE("AppSettingsStore - settings file is not Kemai 0.11's settings.json", "[appSettingsStore][unit]")
{
    QTemporaryDir tempDir;
    REQUIRE(tempDir.isValid());
    PathHelpers::overrideDefaultRootPath(tempDir.path());

    const auto path = PathHelpers::getFilePath(PathHelpers::FilePathType::AppSettings);
    REQUIRE(QFileInfo(path).fileName() == "appSettings.json");
}

TEST_CASE("AppSettingsStore::settingsFromJson - versions", "[appSettingsStore][unit]")
{
    REQUIRE(AppSettingsStore::settingsFromJson(makeSettingsJson("1.0")).autoLogin->host == "https://kimai.example");
    REQUIRE(AppSettingsStore::settingsFromJson(makeSettingsJson("1.4")).autoLogin.has_value());
    REQUIRE_THROWS_AS(AppSettingsStore::settingsFromJson(makeSettingsJson("2.0")), std::runtime_error);
    REQUIRE_THROWS_AS(AppSettingsStore::settingsFromJson(makeSettingsJson(QString())), std::runtime_error);
}

TEST_CASE("AppSettingsStore::save - round-trip", "[appSettingsStore][unit]")
{
    QTemporaryDir tempDir;
    REQUIRE(tempDir.isValid());
    const auto path = tempDir.filePath("appSettings.json");
    writeFile(path, QJsonDocument(makeSettingsJson("1.0")).toJson());

    AppSettingsStore store;
    store.setStorePath(path);
    store.save(AppSettings{});

    const auto saved = QJsonDocument::fromJson(readFile(path)).object();
    REQUIRE(saved.value("version").toString() == "1.3");
    REQUIRE_FALSE(AppSettingsStore::settingsFromJson(saved).autoLogin.has_value());
}

TEST_CASE("AppSettingsStore::save - never overwrites a newer-major file", "[appSettingsStore][unit]")
{
    QTemporaryDir tempDir;
    REQUIRE(tempDir.isValid());
    const auto path = tempDir.filePath("appSettings.json");

    const auto original = QJsonDocument(makeSettingsJson("2.0")).toJson();
    writeFile(path, original);

    AppSettingsStore store;
    store.setStorePath(path);
    store.save(AppSettings{});

    REQUIRE(readFile(path) == original);
}

TEST_CASE("AppSettingsStore::save - never overwrites a corrupt file", "[appSettingsStore][unit]")
{
    QTemporaryDir tempDir;
    REQUIRE(tempDir.isValid());
    const auto path = tempDir.filePath("appSettings.json");

    const QByteArray original = R"({"version": "1.0", "settings": {)";
    writeFile(path, original);

    AppSettingsStore store;
    store.setStorePath(path);
    store.save(AppSettings{});

    REQUIRE(readFile(path) == original);
}

TEST_CASE("AppSettingsStore::save - window geometry round-trip", "[appSettingsStore][unit]")
{
    QTemporaryDir tempDir;
    REQUIRE(tempDir.isValid());
    const auto path = tempDir.filePath("appSettings.json");

    AppSettings settings;
    settings.window = WindowGeometry{.normal = QRect(120, 80, 900, 700), .maximized = true};

    AppSettingsStore store;
    store.setStorePath(path);
    store.save(settings);

    const auto loaded = AppSettingsStore::settingsFromJson(QJsonDocument::fromJson(readFile(path)).object());
    REQUIRE(loaded.window.has_value());
    REQUIRE(loaded.window->normal == QRect(120, 80, 900, 700));
    REQUIRE(loaded.window->maximized);
}

TEST_CASE("AppSettingsStore::settingsFromJson - window geometry across versions", "[appSettingsStore][unit]")
{
    // A 1.0 file has no geometry
    REQUIRE_FALSE(AppSettingsStore::settingsFromJson(makeSettingsJson("1.0")).window.has_value());

    // A later 1.x file is read by the same rules, its unknown fields ignored
    auto json     = makeSettingsJson("1.7");
    auto settings = json["settings"].toObject();
    settings.insert("window", QJsonObject{{"x", 10}, {"y", 20}, {"width", 640}, {"height", 480}, {"maximized", false}});
    settings.insert("someFutureField", true);
    json.insert("settings", settings);

    const auto loaded = AppSettingsStore::settingsFromJson(json);
    REQUIRE(loaded.window->normal == QRect(10, 20, 640, 480));
    REQUIRE(loaded.autoLogin->host == "https://kimai.example");

    // A degenerate size is no geometry
    settings.insert("window", QJsonObject{{"x", 10}, {"y", 20}, {"width", 0}, {"height", 480}});
    json.insert("settings", settings);
    REQUIRE_FALSE(AppSettingsStore::settingsFromJson(json).window.has_value());
}

TEST_CASE("AppSettingsStore::save - preferences unknown to this build are kept", "[appSettingsStore][unit]")
{
    QTemporaryDir tempDir;
    REQUIRE(tempDir.isValid());
    const auto path = tempDir.filePath("appSettings.json");

    // Written by a newer 1.x Kemai
    auto json     = makeSettingsJson("1.9");
    auto settings = json["settings"].toObject();
    settings.insert("preferences", QJsonObject{{"someFuturePreference", 42}, {"anotherOne", "x"}});
    json.insert("settings", settings);
    writeFile(path, QJsonDocument(json).toJson());

    AppSettingsStore store;
    store.setStorePath(path);
    REQUIRE(store.canSave());

    // This build changes something else and saves what it read
    auto read = store.readFile();
    read.window = WindowGeometry{.normal = QRect(0, 0, 800, 600)};
    read.autoLogin.reset(); // no keychain write in tests
    store.save(read);

    const auto saved = AppSettingsStore::settingsFromJson(QJsonDocument::fromJson(readFile(path)).object());
    REQUIRE(saved.preferences.value("someFuturePreference").toInt() == 42);
    REQUIRE(saved.preferences.value("anotherOne").toString() == "x");
}

TEST_CASE("AppSettingsStore::readFile - file without preferences", "[appSettingsStore][unit]")
{
    REQUIRE(AppSettingsStore::settingsFromJson(makeSettingsJson("1.1")).preferences.isEmpty());
}

TEST_CASE("AppSettingsStore::canSave - only a missing or readable file", "[appSettingsStore][unit]")
{
    QTemporaryDir tempDir;
    REQUIRE(tempDir.isValid());
    const auto path = tempDir.filePath("appSettings.json");

    AppSettingsStore store;
    store.setStorePath(path);
    REQUIRE(store.canSave()); // missing

    writeFile(path, QJsonDocument(makeSettingsJson("1.0")).toJson());
    REQUIRE(store.canSave());

    const QByteArray corrupt = R"({"version": "1.0", "settings": {)";
    writeFile(path, corrupt);
    REQUIRE_FALSE(store.canSave());
    REQUIRE(store.readFile().preferences.isEmpty()); // defaults
    store.save(AppSettings{});
    REQUIRE(readFile(path) == corrupt);

    writeFile(path, QJsonDocument(makeSettingsJson("2.0")).toJson());
    REQUIRE_FALSE(store.canSave());
}

TEST_CASE("AppSettingsStore::save - trusted certificates round-trip", "[appSettingsStore][unit]")
{
    QTemporaryDir tempDir;
    REQUIRE(tempDir.isValid());
    const auto path = tempDir.filePath("appSettings.json");

    const TrustedCertificate certificate{.host      = "kimai.example.lan:8443",
                                         .sha256    = "AB:CD:EF",
                                         .pem       = "-----BEGIN CERTIFICATE-----\nMIIB\n-----END CERTIFICATE-----\n",
                                         .expiresAt = QDateTime(QDate(2027, 3, 12), QTime(10, 0), QTimeZone::UTC)};
    AppSettings settings;
    settings.trustedCertificates = {certificate};

    AppSettingsStore store;
    store.setStorePath(path);
    store.save(settings);

    const auto loaded = AppSettingsStore::settingsFromJson(QJsonDocument::fromJson(readFile(path)).object());
    REQUIRE(loaded.trustedCertificates.size() == 1);
    REQUIRE(loaded.trustedCertificates[0].host == certificate.host);
    REQUIRE(loaded.trustedCertificates[0].sha256 == certificate.sha256);
    REQUIRE(loaded.trustedCertificates[0].pem == certificate.pem);
    REQUIRE(loaded.trustedCertificates[0].expiresAt == certificate.expiresAt);
}
