// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include <catch2/catch_test_macros.hpp>

#include <QCoreApplication>
#include <QDateTime>

#include <context/sessionController.h>

#include "../mocks/fakeKimaiClient.h"

using namespace kemai;

namespace
{
    void processEvents(int rounds = 10)
    {
        for (int i = 0; i < rounds; ++i)
        {
            QCoreApplication::processEvents();
        }
    }

    KimaiTimeSheet makeTimeSheet(int id, const QDateTime &beginAt, const QDateTime &endAt = {})
    {
        KimaiTimeSheet ts;
        ts.id            = id;
        ts.activity.id   = 11;
        ts.activity.name = "Design";
        ts.project.id    = 7;
        ts.project.name  = "Rocket";
        ts.beginAt       = beginAt;
        ts.endAt         = endAt;
        return ts;
    }

    struct SignalProbe
    {
        int sessionExpiredCount = 0;
        int notificationCount   = 0;

        SessionController::NotificationLevel lastLevel = SessionController::NotificationLevel::Info;
        SessionController::NotificationKind  lastKind  = SessionController::NotificationKind::UnknownError;

        void attach(SessionController &controller)
        {
            QObject::connect(&controller, &SessionController::sessionExpired, [this] { ++sessionExpiredCount; });
            QObject::connect(&controller, &SessionController::notificationRequested,
                             [this](SessionController::NotificationLevel level, SessionController::NotificationKind kind, const QString &)
                             {
                                 ++notificationCount;
                                 lastLevel = level;
                                 lastKind  = kind;
                             });
        }
    };
} // namespace

TEST_CASE("SessionController - nominal start triggers refreshes", "[session][unit]")
{
    auto fake = std::make_shared<FakeKimaiClient>();

    SessionController controller(fake, KimaiUser{});
    processEvents();

    SignalProbe probe;
    probe.attach(controller);

    fake->calls.clear();
    controller.startTimeSheet(11, 7);
    processEvents();

    REQUIRE(fake->calls.contains("startTimeSheet"));
    REQUIRE(fake->calls.contains("requestActiveTimeSheets"));
    REQUIRE(fake->calls.contains("requestTimeSheets"));
    REQUIRE(probe.sessionExpiredCount == 0);
    REQUIRE(probe.notificationCount == 0);
}

TEST_CASE("SessionController - 401 on action emits sessionExpired, no notification", "[session][error][unit]")
{
    auto fake = std::make_shared<FakeKimaiClient>();
    fake->failWith("startTimeSheet", ApiErrorKind::AuthFailed, 401);

    SessionController controller(fake, KimaiUser{});
    processEvents();

    SignalProbe probe;
    probe.attach(controller);

    controller.startTimeSheet(11, 7);
    processEvents();

    REQUIRE(probe.sessionExpiredCount == 1);
    REQUIRE(probe.notificationCount == 0);
}

TEST_CASE("SessionController - network failure on stop notifies the UI", "[session][error][unit]")
{
    auto fake = std::make_shared<FakeKimaiClient>();
    fake->activeTimeSheets.append(makeTimeSheet(42, QDateTime::currentDateTimeUtc().addSecs(-600)));
    fake->failWith("updateTimeSheet", ApiErrorKind::NoNetwork);

    SessionController controller(fake, KimaiUser{});
    processEvents();
    REQUIRE(controller.hasActiveTimesheet());

    SignalProbe probe;
    probe.attach(controller);

    controller.stopActiveTimeSheet();
    processEvents();

    REQUIRE(probe.sessionExpiredCount == 0);
    REQUIRE(probe.notificationCount == 1);
    REQUIRE(probe.lastLevel == SessionController::NotificationLevel::Error);
    REQUIRE(probe.lastKind == SessionController::NotificationKind::NoNetwork);
}

TEST_CASE("SessionController - cache failure notifies CacheLoadFailed", "[session][error][unit]")
{
    auto fake = std::make_shared<FakeKimaiClient>();
    fake->failWith("requestProjects", ApiErrorKind::ServerError, 500);

    SessionController controller(fake, KimaiUser{});

    SignalProbe probe;
    probe.attach(controller);
    processEvents();

    REQUIRE(probe.sessionExpiredCount == 0);
    REQUIRE(probe.notificationCount == 1);
    REQUIRE(probe.lastKind == SessionController::NotificationKind::CacheLoadFailed);
}

TEST_CASE("SessionController - cache failure on 401 expires the session", "[session][error][unit]")
{
    auto fake = std::make_shared<FakeKimaiClient>();
    fake->failWith("requestCustomers", ApiErrorKind::AuthFailed, 401);

    SessionController controller(fake, KimaiUser{});

    SignalProbe probe;
    probe.attach(controller);
    processEvents();

    REQUIRE(probe.sessionExpiredCount == 1);
    REQUIRE(probe.notificationCount == 0);
}

TEST_CASE("SessionController::updateTimeSheet - rejects end before begin", "[session][unit]")
{
    const auto begin = QDateTime::fromString("2026-06-10T10:00:00Z", Qt::ISODate);
    const auto end   = QDateTime::fromString("2026-06-10T12:00:00Z", Qt::ISODate);

    auto fake = std::make_shared<FakeKimaiClient>();
    fake->timeSheets.append(makeTimeSheet(42, begin, end));

    SessionController controller(fake, KimaiUser{});
    processEvents();

    fake->calls.clear();
    controller.updateTimeSheet(42, 7, 11, {}, {}, {}, begin.addSecs(-60));
    processEvents();

    REQUIRE_FALSE(fake->calls.contains("updateTimeSheet"));
}

TEST_CASE("SessionController::updateTimeSheet - invalid datetime leaves field unchanged", "[session][unit]")
{
    const auto begin  = QDateTime::fromString("2026-06-10T10:00:00Z", Qt::ISODate);
    const auto end    = QDateTime::fromString("2026-06-10T12:00:00Z", Qt::ISODate);
    const auto newEnd = QDateTime::fromString("2026-06-10T13:30:00Z", Qt::ISODate);

    auto fake = std::make_shared<FakeKimaiClient>();
    fake->timeSheets.append(makeTimeSheet(42, begin, end));

    SessionController controller(fake, KimaiUser{});
    processEvents();

    controller.updateTimeSheet(42, 7, 11, {}, {}, {}, newEnd);
    processEvents();

    REQUIRE(fake->calls.contains("updateTimeSheet"));
    REQUIRE(fake->lastUpdatedTimeSheet.id == 42);
    REQUIRE(fake->lastUpdatedTimeSheet.beginAt == begin);
    REQUIRE(fake->lastUpdatedTimeSheet.endAt == newEnd.toUTC());
}

TEST_CASE("SessionController::updateTimeSheet - applies project, activity, description and tags", "[session][unit]")
{
    const auto begin = QDateTime::fromString("2026-06-10T10:00:00Z", Qt::ISODate);
    const auto end   = QDateTime::fromString("2026-06-10T12:00:00Z", Qt::ISODate);

    auto fake = std::make_shared<FakeKimaiClient>();
    auto ts   = makeTimeSheet(42, begin, end);
    ts.description = "Old";
    ts.tags        = {"Legacy"};
    fake->timeSheets.append(ts);

    SessionController controller(fake, KimaiUser{});
    processEvents();

    fake->calls.clear();
    controller.updateTimeSheet(42, 8, 12, "New description", {"QML", "Frontend"}, {}, {});
    processEvents();

    REQUIRE(fake->calls.contains("updateTimeSheet"));
    REQUIRE(fake->lastUpdatedTimeSheet.project.id == 8);
    REQUIRE(fake->lastUpdatedTimeSheet.activity.id == 12);
    REQUIRE(fake->lastUpdatedTimeSheet.description == "New description");
    REQUIRE(fake->lastUpdatedTimeSheet.tags == QStringList{"QML", "Frontend"});
    REQUIRE(fake->lastUpdatedTimeSheet.beginAt == begin);
    REQUIRE(fake->lastUpdatedTimeSheet.endAt == end);
    // Success refreshes the lists
    REQUIRE(fake->calls.contains("requestActiveTimeSheets"));
    REQUIRE(fake->calls.contains("requestTimeSheets"));
}

TEST_CASE("SessionController::updateTimeSheet - running entry keeps no end", "[session][unit]")
{
    const auto begin = QDateTime::fromString("2026-06-10T10:00:00Z", Qt::ISODate);

    auto fake = std::make_shared<FakeKimaiClient>();
    fake->timeSheets.append(makeTimeSheet(42, begin));

    SessionController controller(fake, KimaiUser{});
    processEvents();

    controller.updateTimeSheet(42, 7, 11, "Described while running", {}, {}, begin.addSecs(3600));
    processEvents();

    REQUIRE(fake->lastUpdatedTimeSheet.description == "Described while running");
    REQUIRE_FALSE(fake->lastUpdatedTimeSheet.endAt.isValid());
}

TEST_CASE("SessionController::updateTimeSheet - rejects an invalid project or activity", "[session][unit]")
{
    const auto begin = QDateTime::fromString("2026-06-10T10:00:00Z", Qt::ISODate);
    const auto end   = QDateTime::fromString("2026-06-10T12:00:00Z", Qt::ISODate);

    auto fake = std::make_shared<FakeKimaiClient>();
    fake->timeSheets.append(makeTimeSheet(42, begin, end));

    SessionController controller(fake, KimaiUser{});
    processEvents();

    fake->calls.clear();
    controller.updateTimeSheet(42, 0, 11, {}, {}, {}, {});
    processEvents();

    REQUIRE_FALSE(fake->calls.contains("updateTimeSheet"));
}

TEST_CASE("SessionController - failed refresh warns and keeps the activities", "[session][error][unit]")
{
    auto fake = std::make_shared<FakeKimaiClient>();

    KimaiProject project;
    project.id   = 7;
    project.name = "Rocket";
    fake->projects.append(project);

    KimaiActivity activity;
    activity.id   = 11;
    activity.name = "Meeting"; // global: fanned out to "Rocket"
    fake->activities.append(activity);

    SessionController controller(fake, KimaiUser{});
    processEvents();
    REQUIRE(controller.activityModel()->rowCount() == 1);

    SignalProbe probe;
    probe.attach(controller);

    fake->failWith("requestActivities", ApiErrorKind::NoNetwork);
    controller.refresh();
    REQUIRE(controller.isRefreshing());
    processEvents();

    REQUIRE_FALSE(controller.isRefreshing());
    REQUIRE(probe.notificationCount == 1);
    REQUIRE(probe.lastLevel == SessionController::NotificationLevel::Warning);
    REQUIRE(probe.lastKind == SessionController::NotificationKind::CacheRefreshFailed);
    REQUIRE(controller.activityModel()->rowCount() == 1);
}

TEST_CASE("SessionController - 401 during refresh expires the session", "[session][error][unit]")
{
    auto fake = std::make_shared<FakeKimaiClient>();

    SessionController controller(fake, KimaiUser{});
    processEvents();

    SignalProbe probe;
    probe.attach(controller);

    fake->failWith("requestProjects", ApiErrorKind::AuthFailed, 401);
    controller.refresh();
    processEvents();

    REQUIRE(probe.sessionExpiredCount >= 1);
}

TEST_CASE("SessionController::refreshIfStale - only reloads a stale or missing cache", "[session][unit]")
{
    auto fake = std::make_shared<FakeKimaiClient>();

    SessionController controller(fake, KimaiUser{});
    processEvents();

    // Fresh cache: nothing to do
    fake->calls.clear();
    controller.refreshIfStale();
    processEvents();
    REQUIRE_FALSE(fake->calls.contains("requestActivities"));

    // Stale cache: reload cache and timesheets
    controller.setStaleAfter(std::chrono::seconds(0));
    controller.refreshIfStale();
    processEvents();
    REQUIRE(fake->calls.contains("requestActivities"));
    REQUIRE(fake->calls.contains("requestTimeSheets"));
}

TEST_CASE("SessionController::refreshIfStale - retries after a failed initial load", "[session][unit]")
{
    auto fake = std::make_shared<FakeKimaiClient>();
    fake->failWith("requestCustomers", ApiErrorKind::NoNetwork);

    SessionController controller(fake, KimaiUser{});
    processEvents();

    fake->errors.clear();
    fake->calls.clear();
    controller.refreshIfStale();
    processEvents();

    REQUIRE(fake->calls.contains("requestCustomers"));
}

TEST_CASE("SessionController::refresh - coalesces with a cache load in flight", "[session][unit]")
{
    auto fake = std::make_shared<FakeKimaiClient>();

    SessionController controller(fake, KimaiUser{});
    // Initial load still in flight
    fake->calls.clear();
    controller.refresh();
    processEvents();

    REQUIRE_FALSE(fake->calls.contains("requestActivities"));
    REQUIRE(fake->calls.contains("requestTimeSheets"));
}

TEST_CASE("SessionController::deleteTimeSheet - deletes then refreshes the lists", "[session][unit]")
{
    const auto begin = QDateTime::fromString("2026-06-10T10:00:00Z", Qt::ISODate);
    const auto end   = QDateTime::fromString("2026-06-10T12:00:00Z", Qt::ISODate);

    auto fake = std::make_shared<FakeKimaiClient>();
    fake->timeSheets.append(makeTimeSheet(42, begin, end));

    SessionController controller(fake, KimaiUser{});
    processEvents();

    fake->calls.clear();
    controller.deleteTimeSheet(42);
    processEvents();

    REQUIRE(fake->lastDeletedTimeSheetId == 42);
    REQUIRE(fake->calls.indexOf("deleteTimeSheet") < fake->calls.indexOf("requestActiveTimeSheets"));
    REQUIRE(fake->calls.contains("requestTimeSheets"));
}

TEST_CASE("SessionController::deleteTimeSheet - deleting the active entry stops tracking", "[session][unit]")
{
    const auto begin = QDateTime::currentDateTimeUtc().addSecs(-600);

    auto fake = std::make_shared<FakeKimaiClient>();
    fake->activeTimeSheets.append(makeTimeSheet(42, begin));
    fake->timeSheets.append(makeTimeSheet(42, begin));

    SessionController controller(fake, KimaiUser{});
    processEvents();
    REQUIRE(controller.hasActiveTimesheet());

    // The server no longer has it after deletion
    fake->activeTimeSheets.clear();
    fake->timeSheets.clear();
    controller.deleteTimeSheet(42);
    processEvents();

    REQUIRE_FALSE(controller.hasActiveTimesheet());
}

TEST_CASE("SessionController::deleteTimeSheet - 403 is reported to the caller, not the banner", "[session][error][unit]")
{
    auto fake = std::make_shared<FakeKimaiClient>();

    SessionController controller(fake, KimaiUser{});
    processEvents();

    SignalProbe probe;
    probe.attach(controller);

    int  failedId   = 0;
    auto failedKind = SessionController::NotificationKind::UnknownError;
    QObject::connect(&controller, &SessionController::timeSheetDeleteFailed,
                     [&](int id, SessionController::NotificationKind kind, const QString &)
                     {
                         failedId   = id;
                         failedKind = kind;
                     });

    fake->failWith("deleteTimeSheet", ApiErrorKind::Forbidden, 403);
    REQUIRE(controller.deleteTimeSheet(42));
    processEvents();

    REQUIRE(failedId == 42);
    REQUIRE(failedKind == SessionController::NotificationKind::Forbidden);
    REQUIRE(probe.notificationCount == 0);
    REQUIRE(probe.sessionExpiredCount == 0);
}

TEST_CASE("SessionController::deleteTimeSheet - success emits timeSheetDeleted", "[session][unit]")
{
    auto fake = std::make_shared<FakeKimaiClient>();

    SessionController controller(fake, KimaiUser{});
    processEvents();

    int deletedId = 0;
    QObject::connect(&controller, &SessionController::timeSheetDeleted, [&](int id) { deletedId = id; });

    REQUIRE(controller.deleteTimeSheet(42));
    REQUIRE_FALSE(controller.deleteTimeSheet(0));
    processEvents();

    REQUIRE(deletedId == 42);
}

TEST_CASE("SessionController::updateTimeSheet - reports success or failure to the caller", "[session][error][unit]")
{
    const auto begin = QDateTime::fromString("2026-06-10T10:00:00Z", Qt::ISODate);
    const auto end   = QDateTime::fromString("2026-06-10T12:00:00Z", Qt::ISODate);

    auto fake = std::make_shared<FakeKimaiClient>();
    fake->timeSheets.append(makeTimeSheet(42, begin, end));

    SessionController controller(fake, KimaiUser{});
    processEvents();

    SignalProbe probe;
    probe.attach(controller);

    int     savedId  = 0;
    int     failedId = 0;
    auto    kind     = SessionController::NotificationKind::UnknownError;
    QString detail;
    QObject::connect(&controller, &SessionController::timeSheetSaved, [&](int id) { savedId = id; });
    QObject::connect(&controller, &SessionController::timeSheetSaveFailed,
                     [&](int id, SessionController::NotificationKind k, const QString &d)
                     {
                         failedId = id;
                         kind     = k;
                         detail   = d;
                     });

    SECTION("success")
    {
        REQUIRE(controller.updateTimeSheet(42, 7, 11, "desc", {}, {}, {}));
        processEvents();
        REQUIRE(savedId == 42);
        REQUIRE(failedId == 0);
    }

    SECTION("network failure")
    {
        fake->failWith("updateTimeSheet", ApiErrorKind::NoNetwork);
        REQUIRE(controller.updateTimeSheet(42, 7, 11, "desc", {}, {}, {}));
        processEvents();
        REQUIRE(failedId == 42);
        REQUIRE(kind == SessionController::NotificationKind::NoNetwork);
        REQUIRE(detail == "fake error");
        REQUIRE(probe.notificationCount == 0);
    }

    SECTION("rejected locally")
    {
        REQUIRE_FALSE(controller.updateTimeSheet(42, 7, 11, "desc", {}, {}, begin.addSecs(-60)));
        processEvents();
        REQUIRE(savedId == 0);
        REQUIRE(failedId == 0);
    }
}

TEST_CASE("SessionController - punch mode follows the timesheet config", "[session][unit]")
{
    auto fake                          = std::make_shared<FakeKimaiClient>();
    fake->timesheetConfig.trackingMode = KimaiTimeSheetConfig::TrackingMode::Punch;

    SessionController controller(fake, KimaiUser{});
    REQUIRE_FALSE(controller.isPunchMode());
    processEvents();

    REQUIRE(controller.isPunchMode());
}

TEST_CASE("SessionController::deleteTimeSheet - 401 expires the session", "[session][error][unit]")
{
    auto fake = std::make_shared<FakeKimaiClient>();

    SessionController controller(fake, KimaiUser{});
    processEvents();

    SignalProbe probe;
    probe.attach(controller);

    fake->failWith("deleteTimeSheet", ApiErrorKind::AuthFailed, 401);
    controller.deleteTimeSheet(42);
    processEvents();

    REQUIRE(probe.sessionExpiredCount == 1);
    REQUIRE(probe.notificationCount == 0);
}

TEST_CASE("SessionController::requestTagSuggestions - server tags reach the signal with their query", "[session][unit]")
{
    auto fake  = std::make_shared<FakeKimaiClient>();
    fake->tags = {"Frontend", "Front office"};

    SessionController controller(fake, KimaiUser{});
    processEvents();

    QString     receivedQuery;
    QStringList receivedTags;
    QObject::connect(&controller, &SessionController::tagSuggestionsReady,
                     [&](const QString &query, const QStringList &tags)
                     {
                         receivedQuery = query;
                         receivedTags  = tags;
                     });

    controller.requestTagSuggestions("Fro");
    processEvents();

    REQUIRE(fake->lastTagsQuery == "Fro");
    REQUIRE(receivedQuery == "Fro");
    REQUIRE(receivedTags == QStringList{"Frontend", "Front office"});
}

TEST_CASE("SessionController::requestTagSuggestions - failure goes through the error policy", "[session][error][unit]")
{
    auto fake = std::make_shared<FakeKimaiClient>();

    SessionController controller(fake, KimaiUser{});
    processEvents();

    SignalProbe probe;
    probe.attach(controller);

    fake->failWith("requestTags", ApiErrorKind::NoNetwork);
    controller.requestTagSuggestions("Fro");
    processEvents();

    REQUIRE(probe.notificationCount == 1);
    REQUIRE(probe.lastKind == SessionController::NotificationKind::NoNetwork);
}
