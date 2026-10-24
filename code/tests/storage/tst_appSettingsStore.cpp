// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include <catch2/catch_test_macros.hpp>

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>

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
    REQUIRE(saved.value("version").toString() == "1.0");
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
