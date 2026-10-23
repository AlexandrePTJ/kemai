// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include <catch2/catch_test_macros.hpp>

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QUuid>

#include <storage/profileStore.h>

using namespace kemai;

namespace
{
    QJsonObject makeProfilesJson(const QString &version)
    {
        QJsonObject profile{
            {"id",   QUuid::createUuid().toString()},
            {"name", "Home"                        },
            {"host", "https://kimai.example"       }
        };

        QJsonObject json{
            {"profiles", QJsonArray{profile}}
        };
        if (!version.isNull())
        {
            json.insert("version", version);
        }
        return json;
    }

    Profile makeValidProfile()
    {
        return {.id = QUuid::createUuid(), .name = "Work", .host = "https://work.example", .token = "secret"};
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

TEST_CASE("ProfileStore::profilesFromJson - current version is parsed", "[profileStore][unit]")
{
    const auto profiles = ProfileStore::profilesFromJson(makeProfilesJson("1.0"));

    REQUIRE(profiles.size() == 1);
    REQUIRE(profiles.front().name == "Home");
    REQUIRE(profiles.front().host == "https://kimai.example");
}

TEST_CASE("ProfileStore::profilesFromJson - newer minor of the same major is read", "[profileStore][unit]")
{
    auto json = makeProfilesJson("1.3");
    // Additive change from a newer Kemai: unknown fields are ignored
    auto profiles = json["profiles"].toArray();
    auto profile  = profiles.first().toObject();
    profile.insert("color", "#ff0000");
    profiles.replace(0, profile);
    json.insert("profiles", profiles);

    REQUIRE(ProfileStore::profilesFromJson(json).size() == 1);
}

TEST_CASE("ProfileStore::profilesFromJson - newer major is rejected", "[profileStore][unit]")
{
    REQUIRE_THROWS_AS(ProfileStore::profilesFromJson(makeProfilesJson("2.0")), std::runtime_error);
}

TEST_CASE("ProfileStore::profilesFromJson - missing or invalid version is rejected", "[profileStore][unit]")
{
    REQUIRE_THROWS_AS(ProfileStore::profilesFromJson(makeProfilesJson(QString())), std::runtime_error);
    REQUIRE_THROWS_AS(ProfileStore::profilesFromJson(makeProfilesJson("not-a-version")), std::runtime_error);
}

TEST_CASE("ProfileStore::save - never overwrites a newer-major file", "[profileStore][unit]")
{
    QTemporaryDir tempDir;
    REQUIRE(tempDir.isValid());
    const auto path = tempDir.filePath("profiles.json");

    const auto original = QJsonDocument(makeProfilesJson("2.0")).toJson();
    writeFile(path, original);

    ProfileStore store;
    store.setStorePath(path);
    store.save(makeValidProfile());

    REQUIRE(readFile(path) == original);
}

TEST_CASE("ProfileStore::save - never overwrites a corrupt file", "[profileStore][unit]")
{
    QTemporaryDir tempDir;
    REQUIRE(tempDir.isValid());
    const auto path = tempDir.filePath("profiles.json");

    const QByteArray original = R"({"version": "1.0", "profiles": [ {"name": "Home")";
    writeFile(path, original);

    ProfileStore store;
    store.setStorePath(path);
    store.save(makeValidProfile());

    REQUIRE(readFile(path) == original);
}
