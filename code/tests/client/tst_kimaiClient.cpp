// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include <catch2/catch_test_macros.hpp>

#include <QEventLoop>
#include <QFutureWatcher>
#include <QTimer>

#include "client/kimaiApiError.h"
#include "client/kimaiClient.h"

using namespace kemai;

template<typename T>
T waitForFuture(QFuture<T> future, int timeoutMs = 10000)
{
    QFutureWatcher<T> watcher;
    QEventLoop        loop;
    QObject::connect(&watcher, &QFutureWatcherBase::finished, &loop, &QEventLoop::quit);
    QTimer::singleShot(timeoutMs, &loop, &QEventLoop::quit);
    watcher.setFuture(future);
    loop.exec();
    return watcher.result();
}

// Body-less futures: waitForFinished() rethrows the stored KimaiApiError, if any
void waitForFuture(QFuture<void> future, int timeoutMs = 10000)
{
    QFutureWatcher<void> watcher;
    QEventLoop           loop;
    QObject::connect(&watcher, &QFutureWatcherBase::finished, &loop, &QEventLoop::quit);
    QTimer::singleShot(timeoutMs, &loop, &QEventLoop::quit);
    watcher.setFuture(future);
    loop.exec();
    future.waitForFinished();
}

static const QString host  = qEnvironmentVariable("KEMAI_TEST_HOST");
static const QString token = qEnvironmentVariable("KEMAI_TEST_TOKEN");

TEST_CASE("KimaiClient - requestKimaiVersion", "[client][integration]")
{
    if (host.isEmpty() || token.isEmpty())
    {
        SKIP("Set KEMAI_TEST_HOST and KEMAI_TEST_TOKEN to run integration tests");
    }

    KimaiClient client;
    client.setHost(host);
    client.setToken(token);

    auto version = waitForFuture(client.requestKimaiVersion());
    REQUIRE_FALSE(version.kimai.isNull());
}

TEST_CASE("KimaiClient - requestMeUserInfo", "[client][integration]")
{
    if (host.isEmpty() || token.isEmpty())
    {
        SKIP("Set KEMAI_TEST_HOST and KEMAI_TEST_TOKEN to run integration tests");
    }

    KimaiClient client;
    client.setHost(host);
    client.setToken(token);

    auto user = waitForFuture(client.requestMeUserInfo());
    REQUIRE_FALSE(user.username.isEmpty());
}

TEST_CASE("KimaiClient - bad token returns error", "[client][integration]")
{
    if (host.isEmpty())
    {
        SKIP("Set KEMAI_TEST_HOST to run integration tests");
    }

    KimaiClient client;
    client.setHost(host);
    client.setToken("invalid-token");

    REQUIRE_THROWS(waitForFuture(client.requestMeUserInfo()));
}

TEST_CASE("KimaiClient - deleteTimeSheet removes a created timesheet", "[client][integration]")
{
    if (host.isEmpty() || token.isEmpty())
    {
        SKIP("Set KEMAI_TEST_HOST and KEMAI_TEST_TOKEN to run integration tests");
    }

    KimaiClient client;
    client.setHost(host);
    client.setToken(token);

    const auto config = waitForFuture(client.requestTimeSheetConfig());
    if (config.trackingMode == KimaiTimeSheetConfig::TrackingMode::Punch)
    {
        SKIP("Punch mode: begin/end cannot be set to create a finished timesheet");
    }

    // Any activity bound to a project of the instance
    const auto activities = waitForFuture(client.requestActivities());
    const auto it         = std::ranges::find_if(activities, [](const KimaiActivity &a) { return a.project.has_value(); });
    REQUIRE(it != activities.cend());

    KimaiTimeSheet ts;
    ts.project.id  = it->project->id;
    ts.activity.id = it->id;
    ts.description = QStringLiteral("kemai integration test (deleteTimeSheet)");
    ts.beginAt     = QDateTime(QDate::currentDate().addDays(-1), QTime(3, 0)).toUTC();
    ts.endAt       = ts.beginAt.addSecs(5 * 60);

    const auto created = waitForFuture(client.startTimeSheet(ts, config.trackingMode));
    REQUIRE(created.id > 0);

    REQUIRE_NOTHROW(waitForFuture(client.deleteTimeSheet(created.id)));

    // Gone: deleting it again is a NotFound
    try
    {
        waitForFuture(client.deleteTimeSheet(created.id));
        FAIL("Second delete should fail");
    }
    catch (const KimaiApiError &e)
    {
        REQUIRE(e.kind() == ApiErrorKind::NotFound);
    }
}

TEST_CASE("KimaiClient::tagNamesFromJson - accepts tag objects and plain names", "[client][unit]")
{
    const auto objects = QJsonDocument::fromJson(R"([{"id": 1, "name": "QML", "color": null}, {"id": 2, "name": "Frontend"}])");
    REQUIRE(KimaiClient::tagNamesFromJson(objects) == QStringList{"QML", "Frontend"});

    const auto names = QJsonDocument::fromJson(R"(["QML", "Frontend"])");
    REQUIRE(KimaiClient::tagNamesFromJson(names) == QStringList{"QML", "Frontend"});

    REQUIRE_THROWS_AS(KimaiClient::tagNamesFromJson(QJsonDocument::fromJson(R"({"name": "QML"})")), std::runtime_error);
}

TEST_CASE("KimaiClient - requestTags finds existing tags by name", "[client][integration]")
{
    if (host.isEmpty() || token.isEmpty())
    {
        SKIP("Set KEMAI_TEST_HOST and KEMAI_TEST_TOKEN to run integration tests");
    }

    KimaiClient client;
    client.setHost(host);
    client.setToken(token);

    // Any tag of the instance, then search it back by name
    const auto allTags = waitForFuture(client.requestTags(QString()));
    REQUIRE_FALSE(allTags.isEmpty());

    const auto found = waitForFuture(client.requestTags(allTags.first()));
    REQUIRE(found.contains(allTags.first()));
}
