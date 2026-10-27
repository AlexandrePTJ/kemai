// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include <catch2/catch_test_macros.hpp>
#include <spdlog/sinks/ringbuffer_sink.h>
#include <spdlog/spdlog.h>

#include <QEventLoop>
#include <QFile>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QTimer>
#include <QUuid>

#include <context/profileController.h>
#include <data/profile.h>
#include <misc/pathHelpers.h>
#include <model/profile.h>

using namespace kemai;

static bool waitForModelReset(ProfileModel *model, int timeoutMs = 5000)
{
    QEventLoop loop;
    bool       signalReceived = false;
    QObject::connect(model,
                     &QAbstractItemModel::modelReset,
                     &loop,
                     [&loop, &signalReceived]()
                     {
                         signalReceived = true;
                         loop.quit();
                     });
    QTimer::singleShot(timeoutMs, &loop, &QEventLoop::quit);
    loop.exec();
    return signalReceived;
}

static const bool keychainEnabled = !qEnvironmentVariable("KEMAI_TEST_KEYCHAIN").isEmpty();

// =============================================================================
// Profile data struct — unit tests
// =============================================================================

TEST_CASE("Profile::isValid - valid profile", "[data][unit]")
{
    Profile p{QUuid::createUuid(), "Work", "https://kimai.example.com", "secret"};
    REQUIRE(p.isValid());
}

TEST_CASE("Profile::isValid - null id is invalid", "[data][unit]")
{
    Profile p{QUuid{}, "Work", "https://kimai.example.com", "secret"};
    REQUIRE_FALSE(p.isValid());
}

TEST_CASE("Profile::isValid - empty name is invalid", "[data][unit]")
{
    Profile p{QUuid::createUuid(), "", "https://kimai.example.com", "secret"};
    REQUIRE_FALSE(p.isValid());
}

TEST_CASE("Profile::isValid - empty host is invalid", "[data][unit]")
{
    Profile p{QUuid::createUuid(), "Work", "", "secret"};
    REQUIRE_FALSE(p.isValid());
}

TEST_CASE("Profile::isValid - empty token is invalid", "[data][unit]")
{
    Profile p{QUuid::createUuid(), "Work", "https://kimai.example.com", ""};
    REQUIRE_FALSE(p.isValid());
}

TEST_CASE("Profile::toJson - token is excluded", "[data][unit]")
{
    Profile p{QUuid::createUuid(), "Work", "https://kimai.example.com", "mysecret"};
    auto    json = p.toJson();

    REQUIRE(json.contains("id"));
    REQUIRE(json.contains("name"));
    REQUIRE(json.contains("host"));
    REQUIRE_FALSE(json.contains("token"));
    REQUIRE(json["name"].toString() == "Work");
    REQUIRE(json["host"].toString() == "https://kimai.example.com");
}

TEST_CASE("Profile::fromJson - all fields present", "[data][unit]")
{
    auto        id = QUuid::createUuid();
    QJsonObject obj{
        {"id",    id.toString()               },
        {"name",  "Personal"                  },
        {"host",  "https://personal.kimai.org"},
        {"token", "mytoken"                   },
    };
    auto p = Profile::fromJson(obj);

    REQUIRE(p.id == id);
    REQUIRE(p.name == "Personal");
    REQUIRE(p.host == "https://personal.kimai.org");
    REQUIRE(p.token == "mytoken");
}

TEST_CASE("Profile::fromJson - token absent defaults to empty string", "[data][unit]")
{
    QJsonObject obj{
        {"id",   QUuid::createUuid().toString()},
        {"name", "Work"                        },
        {"host", "https://kimai.example.com"   },
    };
    auto p = Profile::fromJson(obj);
    REQUIRE(p.token.isEmpty());
}

TEST_CASE("Profile::fromJson - round-trip preserves non-token fields, token is lost", "[data][unit]")
{
    Profile p{QUuid::createUuid(), "Work", "https://kimai.example.com", "secret"};
    auto    p2 = Profile::fromJson(p.toJson());

    REQUIRE(p2.id == p.id);
    REQUIRE(p2.name == p.name);
    REQUIRE(p2.host == p.host);
    REQUIRE(p2.token.isEmpty()); // toJson intentionally excludes the token
}

// =============================================================================
// ProfileModel — unit tests
// =============================================================================

TEST_CASE("ProfileModel - starts empty", "[model][unit]")
{
    ProfileModel model;
    REQUIRE(model.rowCount({}) == 0);
    REQUIRE(model.getByIndex(0).isEmpty());
    REQUIRE(model.getById(QUuid::createUuid()).isEmpty());
}

TEST_CASE("ProfileModel::setProfiles - updates rowCount", "[model][unit]")
{
    ProfileModel model;
    Profile      p1{QUuid::createUuid(), "Work", "https://kimai.example.com", "tok1"};
    Profile      p2{QUuid::createUuid(), "Home", "https://home.kimai.org", "tok2"};

    model.setProfiles({p1, p2});
    REQUIRE(model.rowCount({}) == 2);
}

TEST_CASE("ProfileModel::setProfiles - replaces previous profiles", "[model][unit]")
{
    ProfileModel model;
    Profile      p1{QUuid::createUuid(), "Work", "https://kimai.example.com", "tok1"};
    Profile      p2{QUuid::createUuid(), "Home", "https://home.kimai.org", "tok2"};
    Profile      p3{QUuid::createUuid(), "Client", "https://client.kimai.org", "tok3"};

    model.setProfiles({p1, p2});
    model.setProfiles({p3});
    REQUIRE(model.rowCount({}) == 1);
}

TEST_CASE("ProfileModel::getByIndex - valid index returns correct data", "[model][unit]")
{
    ProfileModel model;
    Profile      p{QUuid::createUuid(), "Work", "https://kimai.example.com", "tok"};
    model.setProfiles({p});

    auto map = model.getByIndex(0);
    REQUIRE_FALSE(map.isEmpty());
    REQUIRE(map["name"].toString() == "Work");
    REQUIRE(map["host"].toString() == "https://kimai.example.com");
    REQUIRE(map["token"].toString() == "tok");
    REQUIRE(map["id"].value<QUuid>() == p.id);
}

TEST_CASE("ProfileModel::getByIndex - out of bounds returns empty map", "[model][unit]")
{
    ProfileModel model;
    REQUIRE(model.getByIndex(-1).isEmpty());
    REQUIRE(model.getByIndex(0).isEmpty());

    Profile p{QUuid::createUuid(), "Work", "https://kimai.example.com", "tok"};
    model.setProfiles({p});
    REQUIRE(model.getByIndex(1).isEmpty());
}

TEST_CASE("ProfileModel::getById - existing id returns correct data", "[model][unit]")
{
    ProfileModel model;
    Profile      p1{QUuid::createUuid(), "Work", "https://kimai.example.com", "tok1"};
    Profile      p2{QUuid::createUuid(), "Home", "https://home.kimai.org", "tok2"};
    model.setProfiles({p1, p2});

    auto map = model.getById(p2.id);
    REQUIRE_FALSE(map.isEmpty());
    REQUIRE(map["name"].toString() == "Home");
}

TEST_CASE("ProfileModel::getById - nonexistent id returns empty map", "[model][unit]")
{
    ProfileModel model;
    Profile      p{QUuid::createUuid(), "Work", "https://kimai.example.com", "tok"};
    model.setProfiles({p});

    REQUIRE(model.getById(QUuid::createUuid()).isEmpty());
}

TEST_CASE("ProfileModel::setProfiles - emits modelReset synchronously", "[model][unit]")
{
    ProfileModel model;
    int          resetCount = 0;
    QObject::connect(&model, &QAbstractItemModel::modelReset, [&resetCount]()
                     { ++resetCount; });

    Profile p{QUuid::createUuid(), "Work", "https://kimai.example.com", "tok"};
    model.setProfiles({p});
    model.setProfiles({});

    REQUIRE(resetCount == 2);
}

// =============================================================================
// ProfileController — integration tests (real keychain + filesystem)
// =============================================================================

TEST_CASE("ProfileController - starts empty when no file exists", "[context]")
{
    QTemporaryDir tempDir;
    REQUIRE(tempDir.isValid());
    PathHelpers::overrideDefaultRootPath(tempDir.path());

    ProfileController ctrl;
    REQUIRE(ctrl.profileModel()->rowCount({}) == 0);
}

TEST_CASE("ProfileController::upsert - null UUID auto-generates id", "[context][integration]")
{
    if (!keychainEnabled)
        SKIP("Set KEMAI_TEST_KEYCHAIN=1 to run keychain integration tests");

    QTemporaryDir tempDir;
    REQUIRE(tempDir.isValid());
    PathHelpers::overrideDefaultRootPath(tempDir.path());

    ProfileController ctrl;
    ctrl.upsert(QUuid{}, "Work", "https://kimai.example.com", "secret123");

    REQUIRE(waitForModelReset(ctrl.profileModel()));
    REQUIRE(ctrl.profileModel()->rowCount({}) == 1);

    auto map = ctrl.profileModel()->getByIndex(0);
    REQUIRE(map["name"].toString() == "Work");
    REQUIRE(map["host"].toString() == "https://kimai.example.com");
    REQUIRE(map["token"].toString() == "secret123");
    REQUIRE_FALSE(map["id"].value<QUuid>().isNull());
}

TEST_CASE("ProfileController::upsert - explicit UUID is preserved", "[context][integration]")
{
    if (!keychainEnabled)
        SKIP("Set KEMAI_TEST_KEYCHAIN=1 to run keychain integration tests");

    QTemporaryDir tempDir;
    REQUIRE(tempDir.isValid());
    PathHelpers::overrideDefaultRootPath(tempDir.path());

    ProfileId         id = QUuid::createUuid();
    ProfileController ctrl;
    ctrl.upsert(id, "Personal", "https://personal.kimai.org", "mytoken");

    REQUIRE(waitForModelReset(ctrl.profileModel()));
    REQUIRE(ctrl.profileModel()->rowCount({}) == 1);

    auto map = ctrl.profileModel()->getById(id);
    REQUIRE_FALSE(map.isEmpty());
    REQUIRE(map["id"].value<QUuid>() == id);
    REQUIRE(map["token"].toString() == "mytoken");
}

TEST_CASE("ProfileController::upsert - updates existing profile", "[context][integration]")
{
    if (!keychainEnabled)
        SKIP("Set KEMAI_TEST_KEYCHAIN=1 to run keychain integration tests");

    QTemporaryDir tempDir;
    REQUIRE(tempDir.isValid());
    PathHelpers::overrideDefaultRootPath(tempDir.path());

    ProfileId         id = QUuid::createUuid();
    ProfileController ctrl;

    ctrl.upsert(id, "Work", "https://kimai.example.com", "oldtoken");
    REQUIRE(waitForModelReset(ctrl.profileModel()));

    ctrl.upsert(id, "Work Updated", "https://kimai.example.com", "newtoken");
    REQUIRE(waitForModelReset(ctrl.profileModel()));

    REQUIRE(ctrl.profileModel()->rowCount({}) == 1);
    auto map = ctrl.profileModel()->getByIndex(0);
    REQUIRE(map["name"].toString() == "Work Updated");
    REQUIRE(map["token"].toString() == "newtoken");
}

TEST_CASE("ProfileController::upsert - multiple profiles", "[context][integration]")
{
    if (!keychainEnabled)
        SKIP("Set KEMAI_TEST_KEYCHAIN=1 to run keychain integration tests");

    QTemporaryDir tempDir;
    REQUIRE(tempDir.isValid());
    PathHelpers::overrideDefaultRootPath(tempDir.path());

    ProfileId         idA = QUuid::createUuid();
    ProfileId         idB = QUuid::createUuid();
    ProfileController ctrl;

    ctrl.upsert(idA, "Work", "https://kimai.example.com", "tokA");
    REQUIRE(waitForModelReset(ctrl.profileModel()));

    ctrl.upsert(idB, "Home", "https://home.kimai.org", "tokB");
    REQUIRE(waitForModelReset(ctrl.profileModel()));

    REQUIRE(ctrl.profileModel()->rowCount({}) == 2);
    REQUIRE_FALSE(ctrl.profileModel()->getById(idA).isEmpty());
    REQUIRE_FALSE(ctrl.profileModel()->getById(idB).isEmpty());
}

TEST_CASE("ProfileController::remove - removes existing profile", "[context][integration]")
{
    if (!keychainEnabled)
        SKIP("Set KEMAI_TEST_KEYCHAIN=1 to run keychain integration tests");

    QTemporaryDir tempDir;
    REQUIRE(tempDir.isValid());
    PathHelpers::overrideDefaultRootPath(tempDir.path());

    ProfileId         id = QUuid::createUuid();
    ProfileController ctrl;

    ctrl.upsert(id, "Work", "https://kimai.example.com", "secret");
    REQUIRE(waitForModelReset(ctrl.profileModel()));
    REQUIRE(ctrl.profileModel()->rowCount({}) == 1);

    ctrl.remove(id);
    REQUIRE(waitForModelReset(ctrl.profileModel()));
    REQUIRE(ctrl.profileModel()->rowCount({}) == 0);
    REQUIRE(ctrl.profileModel()->getById(id).isEmpty());
}

TEST_CASE("ProfileController - persistence across instances", "[context][integration]")
{
    if (!keychainEnabled)
        SKIP("Set KEMAI_TEST_KEYCHAIN=1 to run keychain integration tests");

    QTemporaryDir tempDir;
    REQUIRE(tempDir.isValid());
    PathHelpers::overrideDefaultRootPath(tempDir.path());

    ProfileId id = QUuid::createUuid();

    {
        ProfileController ctrl1;
        ctrl1.upsert(id, "Work", "https://kimai.example.com", "persistedtoken");
        REQUIRE(waitForModelReset(ctrl1.profileModel()));
    } // ctrl1 destroyed, JSON file and keychain entry persist

    ProfileController ctrl2;
    REQUIRE(waitForModelReset(ctrl2.profileModel())); // JSON exists now — signal fires
    REQUIRE(ctrl2.profileModel()->rowCount({}) == 1);

    auto map = ctrl2.profileModel()->getById(id);
    REQUIRE_FALSE(map.isEmpty());
    REQUIRE(map["name"].toString() == "Work");
    REQUIRE(map["token"].toString() == "persistedtoken");
}

// =============================================================================
// Importing Kemai 0.11 profiles
// =============================================================================

namespace
{
    LegacyProfile makeLegacyProfile(const QString &name, const QString &host, const QString &apiToken = "api-token")
    {
        return {.id = QUuid::createUuid(), .name = name, .host = host, .apiToken = apiToken};
    }

    QByteArray readBytes(const QString &path)
    {
        QFile file(path);
        REQUIRE(file.open(QIODevice::ReadOnly));
        return file.readAll();
    }

    // A temporary root holding the given 0.11 settings sample as settings.json
    void installLegacySettings(const QTemporaryDir &root, const QString &sample)
    {
        REQUIRE(QFile::copy(QString(KEMAI_TEST_DATA_DIR) + "/legacy-settings/" + sample, root.filePath("settings.json")));
        PathHelpers::overrideDefaultRootPath(root.path());
    }
} // namespace

TEST_CASE("ProfileController::legacyCandidatesOf - states and names", "[context][legacyImport][unit]")
{
    const Profile existing{QUuid::createUuid(), "Work", "kimai.example.com/", "token"};
    const Profile other{QUuid::createUuid(), "Home", "https://a.example.com", "token"};

    auto sameId   = makeLegacyProfile("Renamed", "https://elsewhere.example.com");
    sameId.id     = other.id;
    auto sameHost = makeLegacyProfile("Work", "https://kimai.example.com");
    auto clash    = makeLegacyProfile("Home", "https://b.example.com");
    auto noToken  = makeLegacyProfile("Old", "https://old.example.org", "");
    auto fresh    = makeLegacyProfile("Client", "https://client.example.com");

    const auto candidates = ProfileController::legacyCandidatesOf({sameId, sameHost, clash, noToken, fresh}, {existing, other});
    REQUIRE(candidates.size() == 5);

    CHECK(candidates[0].state == LegacyCandidate::State::AlreadyImported);
    CHECK(candidates[1].state == LegacyCandidate::State::AlreadyImported);
    CHECK(candidates[2].state == LegacyCandidate::State::Ready);
    CHECK(candidates[2].finalName == "Home (0.11)");
    CHECK(candidates[3].state == LegacyCandidate::State::NoToken);
    CHECK(candidates[4].state == LegacyCandidate::State::Ready);
    CHECK(candidates[4].finalName == "Client");
}

TEST_CASE("ProfileController - Kemai 0.11 profiles are offered", "[context][legacyImport]")
{
    QTemporaryDir tempDir;
    REQUIRE(tempDir.isValid());
    installLegacySettings(tempDir, "v3.json");

    ProfileController ctrl;
    REQUIRE(ctrl.legacyCandidates().size() == 2);
    CHECK(ctrl.legacyCandidates()[0].state == LegacyCandidate::State::Ready);
    CHECK(ctrl.legacyCandidates()[1].state == LegacyCandidate::State::NoToken);
    CHECK(ctrl.hasLegacyCandidates());
    CHECK(ctrl.legacyCandidatesKnown()); // no profiles file: known at once
    CHECK(ctrl.legacyLastProfileName() == "Work");
}

TEST_CASE("ProfileController - nothing is offered for profiles without API token", "[context][legacyImport]")
{
    QTemporaryDir tempDir;
    REQUIRE(tempDir.isValid());
    installLegacySettings(tempDir, "v2.json");

    ProfileController ctrl;
    REQUIRE(ctrl.legacyCandidates().size() == 1);
    CHECK(ctrl.legacyCandidates()[0].state == LegacyCandidate::State::NoToken);
    CHECK_FALSE(ctrl.hasLegacyCandidates());
    CHECK(ctrl.legacyLastProfileName().isEmpty());
}

TEST_CASE("ProfileController::importLegacyProfiles - failures are reported and nothing is touched", "[context][legacyImport]")
{
    QTemporaryDir tempDir;
    REQUIRE(tempDir.isValid());
    installLegacySettings(tempDir, "v3.json");

    // An unreadable profiles file: every save is refused before reaching the keychain
    const auto profilesPath = tempDir.filePath("profiles.json");
    {
        QFile file(profilesPath);
        REQUIRE(file.open(QIODevice::WriteOnly));
        file.write(R"({"version": "1.0", "profiles": [)");
    }
    const auto profilesBytes = readBytes(profilesPath);
    const auto legacyBytes   = readBytes(tempDir.filePath("settings.json"));

    auto capture  = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(200);
    auto previous = spdlog::default_logger();
    spdlog::set_default_logger(std::make_shared<spdlog::logger>("capture", capture));
    spdlog::set_level(spdlog::level::trace);

    QList<ProfileId>         imported{QUuid::createUuid()};
    QMap<ProfileId, QString> failures;
    {
        ProfileController ctrl;
        QObject::connect(&ctrl, &ProfileController::legacyImportFinished, [&imported, &failures](const QList<ProfileId> &ids, const QMap<ProfileId, QString> &failed)
                         {
                             imported = ids;
                             failures = failed; });
        ctrl.importLegacyProfiles();
        // Still to import: a later start offers it again
        CHECK(ctrl.legacyCandidates()[0].state == LegacyCandidate::State::Ready);
    }

    spdlog::set_default_logger(previous);

    CHECK(imported.isEmpty());
    REQUIRE(failures.size() == 1);
    CHECK(failures.contains(QUuid("{6f1c2b9e-3d4a-4c8b-9e21-5a7d0c3b8f14}")));
    CHECK(readBytes(profilesPath) == profilesBytes);
    CHECK(readBytes(tempDir.filePath("settings.json")) == legacyBytes);
    for (const auto &line : capture->last_formatted())
    {
        CHECK(line.find("example-api-token") == std::string::npos);
    }
}

TEST_CASE("ProfileController::importLegacyProfiles - imports once, token only in the keychain", "[context][legacyImport][integration]")
{
    if (!keychainEnabled)
        SKIP("Set KEMAI_TEST_KEYCHAIN=1 to run keychain integration tests");

    QTemporaryDir tempDir;
    REQUIRE(tempDir.isValid());
    installLegacySettings(tempDir, "v3.json");
    const auto legacyBytes = readBytes(tempDir.filePath("settings.json"));
    const auto workId      = QUuid("{6f1c2b9e-3d4a-4c8b-9e21-5a7d0c3b8f14}");

    {
        ProfileController ctrl;
        QList<ProfileId>  imported;
        QObject::connect(&ctrl, &ProfileController::legacyImportFinished, [&imported](const QList<ProfileId> &ids, const QMap<ProfileId, QString> &)
                         { imported = ids; });
        ctrl.importLegacyProfiles();
        REQUIRE(waitForModelReset(ctrl.profileModel()));

        CHECK(imported == QList<ProfileId>{workId});
        CHECK(ctrl.profileModel()->getById(workId)["token"].toString() == "example-api-token-work");
        CHECK_FALSE(ctrl.hasLegacyCandidates());
    }

    CHECK_FALSE(readBytes(tempDir.filePath("profiles.json")).contains("example-api-token"));
    CHECK(readBytes(tempDir.filePath("settings.json")) == legacyBytes);

    // A second run offers nothing
    ProfileController ctrl;
    REQUIRE(waitForModelReset(ctrl.profileModel()));
    CHECK(ctrl.legacyCandidates()[0].state == LegacyCandidate::State::AlreadyImported);
    CHECK_FALSE(ctrl.hasLegacyCandidates());

    ctrl.remove(workId);
    REQUIRE(waitForModelReset(ctrl.profileModel()));
}
