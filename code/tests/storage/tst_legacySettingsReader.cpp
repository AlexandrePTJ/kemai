// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include <catch2/catch_test_macros.hpp>
#include <spdlog/sinks/ringbuffer_sink.h>
#include <spdlog/spdlog.h>

#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

#include <storage/legacySettingsReader.h>

using namespace kemai;

namespace
{
    QString samplePath(const QString &name)
    {
        return QString(KEMAI_TEST_DATA_DIR) + "/legacy-settings/" + name;
    }

    QByteArray readFile(const QString &path)
    {
        QFile file(path);
        REQUIRE(file.open(QIODevice::ReadOnly));
        return file.readAll();
    }

    // Runs `read` with the default logger replaced by a capture, and returns what was logged
    template<typename F>
    std::vector<std::string> captureLog(F read)
    {
        auto capture  = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(100);
        auto previous = spdlog::default_logger();
        spdlog::set_default_logger(std::make_shared<spdlog::logger>("capture", capture));
        spdlog::set_level(spdlog::level::trace);
        read();
        spdlog::set_default_logger(previous);
        return capture->last_formatted();
    }
} // namespace

TEST_CASE("LegacySettingsReader - version 3 profiles with and without API token", "[legacySettings][unit]")
{
    const auto profiles = LegacySettingsReader::readProfiles(samplePath("v3.json"));

    // The profile without a name is skipped
    REQUIRE(profiles.size() == 2);

    CHECK(profiles[0].id == QUuid("{6f1c2b9e-3d4a-4c8b-9e21-5a7d0c3b8f14}"));
    CHECK(profiles[0].name == "Work");
    CHECK(profiles[0].host == "https://kimai.example.com");
    CHECK(profiles[0].apiToken == "example-api-token-work");
    CHECK(profiles[0].lastConnected);

    CHECK(profiles[1].name == "Old");
    CHECK(profiles[1].apiToken.isEmpty());
    CHECK_FALSE(profiles[1].lastConnected);
}

TEST_CASE("LegacySettingsReader - the legacy API password is never exposed", "[legacySettings][unit]")
{
    for (const auto &name : {"v1.json", "v2.json", "v3.json"})
    {
        for (const auto &profile : LegacySettingsReader::readProfiles(samplePath(name)))
        {
            CHECK(profile.apiToken != "example-legacy-password");
        }
    }
}

TEST_CASE("LegacySettingsReader - versions before 3 have no API token", "[legacySettings][unit]")
{
    const auto v2 = LegacySettingsReader::readProfiles(samplePath("v2.json"));
    REQUIRE(v2.size() == 1);
    CHECK(v2[0].name == "Home");
    CHECK(v2[0].apiToken.isEmpty());
    CHECK(v2[0].lastConnected);

    const auto v1 = LegacySettingsReader::readProfiles(samplePath("v1.json"));
    REQUIRE(v1.size() == 1);
    CHECK(v1[0].name == "default");
    CHECK(v1[0].apiToken.isEmpty());
    CHECK_FALSE(v1[0].lastConnected);
}

TEST_CASE("LegacySettingsReader - no profile from a missing, corrupt or unversioned file", "[legacySettings][unit]")
{
    std::vector<LegacyProfile> profiles{LegacyProfile{}};

    const auto log = captureLog(
        [&profiles]
        {
            CHECK(LegacySettingsReader::readProfiles(samplePath("missing.json")).empty());
            CHECK(LegacySettingsReader::readProfiles(samplePath("no-version.json")).empty());
            profiles = LegacySettingsReader::readProfiles(samplePath("corrupt.json"));
        });

    CHECK(profiles.empty());
    // The reason is logged, and no token with it
    const auto corruptLogged = std::ranges::any_of(log, [](const std::string &line)
                                                   { return line.find("corrupt.json not read") != std::string::npos; });
    CHECK(corruptLogged);
    for (const auto &line : log)
    {
        CHECK(line.find("example-api-token") == std::string::npos);
    }
}

TEST_CASE("LegacySettingsReader - the file is left untouched", "[legacySettings][unit]")
{
    QTemporaryDir tempDir;
    REQUIRE(tempDir.isValid());
    const auto path = tempDir.filePath("settings.json");
    REQUIRE(QFile::copy(samplePath("v3.json"), path));

    const auto bytes    = readFile(path);
    const auto modified = QFileInfo(path).lastModified();

    REQUIRE(LegacySettingsReader::readProfiles(path).size() == 2);

    CHECK(readFile(path) == bytes);
    CHECK(QFileInfo(path).lastModified() == modified);
}
