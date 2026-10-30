// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include <optional>

#include <catch2/catch_test_macros.hpp>

#include <QCoreApplication>
#include <QDateTime>
#include <QElapsedTimer>

#include "../mocks/fakeKimaiClient.h"
#include <context/sessionController.h>

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
            QObject::connect(&controller, &SessionController::sessionExpired, [this]
                             { ++sessionExpiredCount; });
            QObject::connect(&controller,
                             &SessionController::notificationRequested,
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

TEST_CASE("SessionController - week totals come from a request covering the week", "[session][unit]")
{
    const auto todayStart = QDate::currentDate().startOfDay();

    KimaiTimeSheet listed;
    listed.id      = 1;
    listed.beginAt = todayStart;
    listed.endAt   = todayStart.addSecs(3600);

    KimaiTimeSheet unlisted = listed;
    unlisted.id             = 2;
    unlisted.beginAt        = todayStart.addSecs(7200);
    unlisted.endAt          = todayStart.addSecs(10800);

    auto fake            = std::make_shared<FakeKimaiClient>();
    fake->timeSheets     = {listed};
    fake->weekTimeSheets = KimaiTimeSheets{listed, unlisted};

    SessionController controller(fake, KimaiUser{});
    processEvents();

    REQUIRE(fake->calls.contains("requestTimeSheetsSince"));
    REQUIRE(fake->lastTimeSheetsSince == TimesheetModel::totalsPeriodStart());
    REQUIRE(controller.recentTimeSheets()->rowCount() == 1);
    REQUIRE(controller.recentTimeSheets()->todayDuration() == "2h 00m");
    REQUIRE(controller.recentTimeSheets()->weekDuration() == "2h 00m");
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
    fake->failWith("stopTimeSheet", ApiErrorKind::NoNetwork);

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

    auto fake      = std::make_shared<FakeKimaiClient>();
    auto ts        = makeTimeSheet(42, begin, end);
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
    QObject::connect(&controller,
                     &SessionController::timeSheetDeleteFailed,
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
    QObject::connect(&controller, &SessionController::timeSheetDeleted, [&](int id)
                     { deletedId = id; });

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
    QObject::connect(&controller, &SessionController::timeSheetSaved, [&](int id)
                     { savedId = id; });
    QObject::connect(&controller,
                     &SessionController::timeSheetSaveFailed,
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

TEST_CASE("SessionController::updateTimeSheet - creates new tags before updating", "[session][unit]")
{
    const auto begin = QDateTime::fromString("2026-06-10T10:00:00Z", Qt::ISODate);
    const auto end   = QDateTime::fromString("2026-06-10T12:00:00Z", Qt::ISODate);

    auto fake = std::make_shared<FakeKimaiClient>();
    fake->timeSheets.append(makeTimeSheet(42, begin, end));

    SessionController controller(fake, KimaiUser{});
    processEvents();

    int  failedId = 0;
    auto kind     = SessionController::NotificationKind::UnknownError;
    QObject::connect(&controller,
                     &SessionController::timeSheetSaveFailed,
                     [&](int id, SessionController::NotificationKind k, const QString &)
                     {
                         failedId = id;
                         kind     = k;
                     });

    fake->calls.clear();

    SECTION("created then sent; a new tag removed meanwhile is not created")
    {
        REQUIRE(controller.updateTimeSheet(42, 7, 11, {}, {"QML", "perf"}, {}, {}, {"perf", "dropped"}));
        processEvents(20);
        REQUIRE(fake->createdTags == QStringList{"perf"});
        REQUIRE(fake->calls.indexOf("createTag") < fake->calls.indexOf("updateTimeSheet"));
        REQUIRE(fake->lastUpdatedTimeSheet.tags == QStringList{"QML", "perf"});
    }

    SECTION("an existing name (400) does not block the save")
    {
        fake->failWith("createTag", ApiErrorKind::Unknown, 400);
        REQUIRE(controller.updateTimeSheet(42, 7, 11, {}, {"perf"}, {}, {}, {"perf"}));
        processEvents(20);
        REQUIRE(fake->calls.contains("updateTimeSheet"));
        REQUIRE(failedId == 0);
    }

    SECTION("no right to create tags fails the save, nothing updated")
    {
        fake->failWith("createTag", ApiErrorKind::Forbidden, 403);
        REQUIRE(controller.updateTimeSheet(42, 7, 11, {}, {"perf"}, {}, {}, {"perf"}));
        processEvents(20);
        REQUIRE_FALSE(fake->calls.contains("updateTimeSheet"));
        REQUIRE(failedId == 42);
        REQUIRE(kind == SessionController::NotificationKind::Forbidden);
    }
}

TEST_CASE("SessionController::accountName - profile name, else the Kimai user name", "[session][unit]")
{
    auto      fake = std::make_shared<FakeKimaiClient>();
    KimaiUser user;
    user.username = "susan";

    SessionController controller(fake, user);
    REQUIRE(controller.accountName() == "susan");

    controller.setAccount("KimaiDev", "https://demo.kimai.org");
    REQUIRE(controller.accountName() == "KimaiDev");
    REQUIRE(controller.host() == "https://demo.kimai.org");
}

TEST_CASE("SessionController::webUrl - the instance's web interface, https when typed without scheme", "[session][unit]")
{
    SessionController controller(std::make_shared<FakeKimaiClient>(), KimaiUser{});

    controller.setAccount("", "demo.kimai.org");
    REQUIRE(controller.webUrl() == QUrl("https://demo.kimai.org"));

    controller.setAccount("", "http://kimai.lan:8001/");
    REQUIRE(controller.webUrl() == QUrl("http://kimai.lan:8001"));
}

TEST_CASE("SessionController::timeSheetWebUrl - the entry's page, in the user's language", "[session][unit]")
{
    SessionController controller(std::make_shared<FakeKimaiClient>(), KimaiUser{.language = "de"});
    controller.setAccount("", "https://kimai.lan/sub");
    REQUIRE(controller.timeSheetWebUrl(12) == QUrl("https://kimai.lan/sub/de/timesheet/12/edit"));

    SessionController noLanguage(std::make_shared<FakeKimaiClient>(), KimaiUser{});
    noLanguage.setAccount("", "demo.kimai.org");
    REQUIRE(noLanguage.timeSheetWebUrl(11517) == QUrl("https://demo.kimai.org/en/timesheet/11517/edit"));
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
    QObject::connect(&controller,
                     &SessionController::tagSuggestionsReady,
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

TEST_CASE("SessionController::syncActiveTimeSheet - follows a timer started or stopped elsewhere", "[session][sync][unit]")
{
    auto fake = std::make_shared<FakeKimaiClient>();

    SessionController controller(fake, KimaiUser{});
    processEvents();
    REQUIRE_FALSE(controller.hasActiveTimesheet());

    // Started from the web interface
    fake->activeTimeSheets = {makeTimeSheet(5, QDateTime::currentDateTimeUtc().addSecs(-60))};
    fake->calls.clear();
    controller.syncActiveTimeSheet();
    processEvents();

    REQUIRE(controller.hasActiveTimesheet());
    REQUIRE(controller.activeProjectName() == "Rocket");
    REQUIRE(fake->calls.count("requestTimeSheets") == 1);

    // Stopped from another client
    fake->activeTimeSheets.clear();
    fake->calls.clear();
    controller.syncActiveTimeSheet();
    processEvents();

    REQUIRE_FALSE(controller.hasActiveTimesheet());
    REQUIRE(fake->calls.count("requestTimeSheets") == 1);
}

TEST_CASE("SessionController::syncActiveTimeSheet - unchanged timer reloads nothing", "[session][sync][unit]")
{
    auto fake              = std::make_shared<FakeKimaiClient>();
    fake->activeTimeSheets = {makeTimeSheet(5, QDateTime::currentDateTimeUtc().addSecs(-60))};

    SessionController controller(fake, KimaiUser{});
    processEvents();

    int refreshingChanges = 0;
    QObject::connect(&controller, &SessionController::refreshingChanged, [&]
                     { ++refreshingChanges; });

    fake->calls.clear();
    controller.syncActiveTimeSheet();
    processEvents();

    REQUIRE(fake->calls == QStringList{"requestActiveTimeSheets"});
    REQUIRE(refreshingChanges == 0);
    REQUIRE(controller.hasActiveTimesheet());
}

TEST_CASE("SessionController::syncActiveTimeSheet - an edit of the running entry elsewhere is a change", "[session][sync][unit]")
{
    auto fake              = std::make_shared<FakeKimaiClient>();
    fake->activeTimeSheets = {makeTimeSheet(5, QDateTime::currentDateTimeUtc().addSecs(-60))};

    SessionController controller(fake, KimaiUser{});
    processEvents();

    fake->activeTimeSheets.first().description = "Edited on the web";
    fake->calls.clear();
    controller.syncActiveTimeSheet();
    processEvents();

    REQUIRE(fake->calls.count("requestTimeSheets") == 1);
}

TEST_CASE("SessionController::syncActiveTimeSheet - reply older than a local action is dropped", "[session][sync][unit]")
{
    auto fake              = std::make_shared<FakeKimaiClient>();
    fake->activeTimeSheets = {makeTimeSheet(5, QDateTime::currentDateTimeUtc().addSecs(-60))};

    SessionController controller(fake, KimaiUser{});
    processEvents();
    REQUIRE(controller.hasActiveTimesheet());

    // The check leaves first, then the user stops: only the stop's own refresh may update the lists
    fake->activeTimeSheets.clear();
    fake->calls.clear();
    controller.syncActiveTimeSheet();
    controller.stopActiveTimeSheet();
    processEvents();

    REQUIRE_FALSE(controller.hasActiveTimesheet());
    REQUIRE(fake->calls.count("requestTimeSheets") == 1);
}

TEST_CASE("SessionController::syncActiveTimeSheet - a check in flight is not restarted", "[session][sync][unit]")
{
    auto fake = std::make_shared<FakeKimaiClient>();

    SessionController controller(fake, KimaiUser{});
    processEvents();

    fake->calls.clear();
    controller.syncActiveTimeSheet();
    controller.syncActiveTimeSheet();
    processEvents();

    REQUIRE(fake->calls.count("requestActiveTimeSheets") == 1);
}

TEST_CASE("SessionController::syncActiveTimeSheet - runs periodically", "[session][sync][unit]")
{
    auto fake = std::make_shared<FakeKimaiClient>();

    SessionController controller(fake, KimaiUser{});
    controller.setSyncInterval(std::chrono::milliseconds(20));
    processEvents();

    fake->calls.clear();
    QElapsedTimer elapsed;
    elapsed.start();
    while (fake->calls.count("requestActiveTimeSheets") < 2 && elapsed.elapsed() < 2000)
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
    }

    REQUIRE(fake->calls.count("requestActiveTimeSheets") >= 2);
}

TEST_CASE("SessionController::syncActiveTimeSheet - failures are reported once until a success", "[session][sync][error][unit]")
{
    auto fake = std::make_shared<FakeKimaiClient>();

    SessionController controller(fake, KimaiUser{});
    processEvents();

    SignalProbe probe;
    probe.attach(controller);

    fake->failWith("requestActiveTimeSheets", ApiErrorKind::NoNetwork);
    for (int i = 0; i < 3; ++i)
    {
        controller.syncActiveTimeSheet();
        processEvents();
    }

    REQUIRE(probe.notificationCount == 1);
    REQUIRE(probe.lastLevel == SessionController::NotificationLevel::Warning);
    REQUIRE(probe.lastKind == SessionController::NotificationKind::NoNetwork);

    // Back online, then lost again: a new warning
    fake->errors.clear();
    controller.syncActiveTimeSheet();
    processEvents();
    fake->failWith("requestActiveTimeSheets", ApiErrorKind::NoNetwork);
    controller.syncActiveTimeSheet();
    processEvents();

    REQUIRE(probe.notificationCount == 2);
    REQUIRE(probe.sessionExpiredCount == 0);
}

TEST_CASE("SessionController::syncActiveTimeSheet - 401 expires the session", "[session][sync][error][unit]")
{
    auto fake = std::make_shared<FakeKimaiClient>();

    SessionController controller(fake, KimaiUser{});
    processEvents();

    SignalProbe probe;
    probe.attach(controller);

    fake->failWith("requestActiveTimeSheets", ApiErrorKind::AuthFailed, 401);
    controller.syncActiveTimeSheet();
    processEvents();

    REQUIRE(probe.sessionExpiredCount == 1);
    REQUIRE(probe.notificationCount == 0);
}

TEST_CASE("SessionController::stopActiveTimeSheet - stops through the stop endpoint", "[session][unit]")
{
    auto fake = std::make_shared<FakeKimaiClient>();
    fake->activeTimeSheets.append(makeTimeSheet(42, QDateTime::currentDateTimeUtc().addSecs(-600)));

    SessionController controller(fake, KimaiUser{});
    processEvents();

    // Also in punch mode, where an update cannot carry an end
    fake->calls.clear();
    controller.stopActiveTimeSheet();
    processEvents();

    REQUIRE(fake->lastStoppedTimeSheetId == 42);
    REQUIRE_FALSE(fake->calls.contains("updateTimeSheet"));
}

TEST_CASE("SessionController::stopActiveTimeSheetAt - ends the entry when the user left", "[session][away][unit]")
{
    const auto begin = QDateTime::currentDateTimeUtc().addSecs(-3600);
    const auto left  = begin.addSecs(1800);

    auto fake = std::make_shared<FakeKimaiClient>();
    fake->activeTimeSheets.append(makeTimeSheet(42, begin));
    fake->timeSheetResult = makeTimeSheet(42, begin, left);

    SessionController controller(fake, KimaiUser{});
    processEvents();

    std::optional<SessionController::AwayReason> reason;
    QDateTime                                    reportedEnd;
    QObject::connect(&controller,
                     &SessionController::timeSheetStoppedAway,
                     [&](SessionController::AwayReason r, const QDateTime &end)
                     {
                         reason      = r;
                         reportedEnd = end;
                     });

    fake->activeTimeSheets.clear();
    controller.stopActiveTimeSheetAt(left, SessionController::AwayReason::Idle);
    processEvents();

    REQUIRE(fake->calls.contains("updateTimeSheet"));
    REQUIRE(fake->lastUpdatedTimeSheet.id == 42);
    REQUIRE(fake->lastUpdatedTimeSheet.endAt == left);
    REQUIRE(fake->lastStoppedTimeSheetId == 0);
    REQUIRE(reason == SessionController::AwayReason::Idle);
    REQUIRE(reportedEnd == left);
    REQUIRE_FALSE(controller.hasActiveTimesheet());
}

TEST_CASE("SessionController::stopActiveTimeSheetAt - plain stop in punch mode", "[session][away][unit]")
{
    const auto begin = QDateTime::currentDateTimeUtc().addSecs(-3600);

    auto fake = std::make_shared<FakeKimaiClient>();
    fake->activeTimeSheets.append(makeTimeSheet(42, begin));
    fake->timesheetConfig.trackingMode = KimaiTimeSheetConfig::TrackingMode::Punch;

    SessionController controller(fake, KimaiUser{});
    processEvents();
    REQUIRE(controller.isPunchMode());

    int stopped = 0;
    QObject::connect(&controller, &SessionController::timeSheetStoppedAway, [&]
                     { ++stopped; });

    controller.stopActiveTimeSheetAt(begin.addSecs(600), SessionController::AwayReason::Locked);
    processEvents();

    REQUIRE(fake->lastStoppedTimeSheetId == 42);
    REQUIRE_FALSE(fake->calls.contains("updateTimeSheet"));
    REQUIRE(stopped == 1);
}

TEST_CASE("SessionController::stopActiveTimeSheetAt - end before the begin is a plain stop", "[session][away][unit]")
{
    // Timer started from the web while the user was already away
    const auto begin = QDateTime::currentDateTimeUtc().addSecs(-60);

    auto fake = std::make_shared<FakeKimaiClient>();
    fake->activeTimeSheets.append(makeTimeSheet(42, begin));

    SessionController controller(fake, KimaiUser{});
    processEvents();

    controller.stopActiveTimeSheetAt(begin.addSecs(-300), SessionController::AwayReason::Idle);
    processEvents();

    REQUIRE(fake->lastStoppedTimeSheetId == 42);
    REQUIRE_FALSE(fake->calls.contains("updateTimeSheet"));
}

TEST_CASE("SessionController::stopActiveTimeSheetAt - failure keeps the timer and notifies", "[session][away][error][unit]")
{
    const auto begin = QDateTime::currentDateTimeUtc().addSecs(-3600);

    auto fake = std::make_shared<FakeKimaiClient>();
    fake->activeTimeSheets.append(makeTimeSheet(42, begin));
    fake->failWith("updateTimeSheet", ApiErrorKind::NoNetwork);

    SessionController controller(fake, KimaiUser{});
    processEvents();

    SignalProbe probe;
    probe.attach(controller);
    int stopped = 0;
    QObject::connect(&controller, &SessionController::timeSheetStoppedAway, [&]
                     { ++stopped; });

    controller.stopActiveTimeSheetAt(begin.addSecs(600), SessionController::AwayReason::Idle);
    processEvents();

    REQUIRE(stopped == 0);
    REQUIRE(controller.hasActiveTimesheet());
    REQUIRE(probe.notificationCount == 1);
    REQUIRE(probe.lastKind == SessionController::NotificationKind::NoNetwork);
}

TEST_CASE("SessionController::create* - creates and reports the new id, without reloading", "[session][create][unit]")
{
    auto fake = std::make_shared<FakeKimaiClient>();

    KimaiUser user;
    user.language = "fr";
    SessionController controller(fake, user);
    processEvents();

    QList<std::pair<SessionController::ResourceKind, int>> created;
    QObject::connect(&controller, &SessionController::resourceCreated, [&](SessionController::ResourceKind kind, int id)
                     { created.append({kind, id}); });

    fake->calls.clear();
    controller.createCustomer("  Acme  ", "FR", "EUR", "Europe/Paris");
    processEvents();
    controller.createProject("Website", 100);
    processEvents();
    controller.createActivity("Code review", 101);
    processEvents();
    controller.createActivity("Meetings", 0);
    processEvents();

    REQUIRE(created.size() == 4);
    REQUIRE(created[0] == std::pair{SessionController::ResourceKind::Customer, 100});
    REQUIRE(created[2] == std::pair{SessionController::ResourceKind::Activity, 102});
    REQUIRE(fake->lastAddedCustomer.name == "Acme");
    REQUIRE(fake->lastAddedCustomer.countryKey == "FR");
    REQUIRE(fake->lastAddedCustomer.language == "fr");
    REQUIRE(fake->lastAddedProject.customer.id == 100);
    REQUIRE_FALSE(fake->lastAddedActivity.project.has_value()); // global
    REQUIRE_FALSE(fake->calls.contains("requestActivities"));

    controller.reloadResources();
    processEvents();
    REQUIRE(fake->calls.count("requestActivities") == 1);
}

TEST_CASE("SessionController::create* - refusals go to the form, 401 expires the session", "[session][create][error][unit]")
{
    auto fake = std::make_shared<FakeKimaiClient>();

    SessionController controller(fake, KimaiUser{});
    processEvents();

    SignalProbe probe;
    probe.attach(controller);

    std::optional<SessionController::NotificationKind> failure;
    QString                                            detail;
    QString                                            field;
    QObject::connect(&controller,
                     &SessionController::resourceCreateFailed,
                     [&](SessionController::ResourceKind, SessionController::NotificationKind kind, const QString &d, const QString &f)
                     {
                         failure = kind;
                         detail  = d;
                         field   = f;
                     });

    fake->failWith("addProject", ApiErrorKind::Forbidden, 403);
    controller.createProject("Website", 100);
    processEvents();
    REQUIRE(failure == SessionController::NotificationKind::Forbidden);

    fake->errors.insert("addCustomer", KimaiApiError{ApiErrorKind::InvalidValue, 400, "customers", "Please select a valid country.", "country"});
    controller.createCustomer("Acme", "XX", "EUR", "Europe/Paris");
    processEvents();
    REQUIRE(failure == SessionController::NotificationKind::InvalidValue);
    REQUIRE(detail == "Please select a valid country.");
    REQUIRE(field == "country");
    REQUIRE(probe.notificationCount == 0); // the form shows it, not the banner

    fake->failWith("addActivity", ApiErrorKind::AuthFailed, 401);
    controller.createActivity("Code review", 101);
    processEvents();
    REQUIRE(probe.sessionExpiredCount == 1);
}

TEST_CASE("SessionController - choices and duplicate lookup for the creation forms", "[session][create][unit]")
{
    auto fake = std::make_shared<FakeKimaiClient>();

    KimaiCustomer acme;
    acme.id   = 1;
    acme.name = "Acme";
    KimaiCustomer beta;
    beta.id         = 2;
    beta.name       = "beta corp";
    fake->customers = {acme, beta};

    KimaiProject website;
    website.id          = 10;
    website.name        = "Website";
    website.customer.id = 1;
    KimaiProject internal;
    internal.id          = 11;
    internal.name        = "Internal";
    internal.customer.id = 1;
    fake->projects       = {website, internal};

    KimaiActivity design;
    design.id      = 100;
    design.name    = "Design";
    design.project = website;
    KimaiActivity meeting; // global
    meeting.id       = 200;
    meeting.name     = "Meeting";
    meeting.color    = "#8b5cf6";
    fake->activities = {design, meeting};

    SessionController controller(fake, KimaiUser{});
    processEvents();

    const auto customers = controller.customerChoices();
    REQUIRE(customers.size() == 2);
    REQUIRE(customers[0].toMap().value("name") == "Acme");
    REQUIRE(customers[0].toMap().value("projectCount") == 2);

    const auto projects = controller.projectChoices();
    REQUIRE(projects.size() == 2);
    REQUIRE(projects[0].toMap().value("name") == "Internal");
    REQUIRE(projects[0].toMap().value("customerName") == "Acme");

    // Global activities once, though the cache fans them out per project
    const auto globals = controller.globalActivityChoices();
    REQUIRE(globals.size() == 1);
    REQUIRE(globals[0].toMap().value("id") == 200);

    REQUIRE(controller.findCustomer("  ACME ").value("id") == 1);
    REQUIRE(controller.findCustomer("Nobody").isEmpty());
    REQUIRE(controller.findProject("website", 1).value("id") == 10);
    REQUIRE(controller.findProject("website", 2).isEmpty());
    REQUIRE(controller.findActivity("design", 10).value("id") == 100);
    REQUIRE(controller.findActivity("design", 11).isEmpty());
    REQUIRE(controller.findActivity("meeting", 0).value("id") == 200);
    REQUIRE(controller.findActivity("meeting", 10).isEmpty()); // a global one is not "in" the project

    const auto globalPair = controller.pair(11, 200);
    REQUIRE(globalPair.value("projectName") == "Internal");
    REQUIRE(globalPair.value("activityName") == "Meeting");
    REQUIRE(globalPair.value("isGlobal").toBool());
    REQUIRE(controller.pair(11, 100).isEmpty());
}

namespace
{
    KimaiPlugins taskManagementPlugin()
    {
        KimaiPlugin plugin;
        plugin.name      = "TaskManagementBundle";
        plugin.apiPlugin = ApiPlugin::TaskManagement;
        return {plugin};
    }

    KimaiTask openTask(int id, const QString &title)
    {
        KimaiTask task;
        task.id          = id;
        task.title       = title;
        task.status      = KimaiTask::Status::Pending;
        task.project.id  = 7;
        task.activity.id = 11;
        return task;
    }
} // namespace

TEST_CASE("SessionController - tasks only with the plugin", "[session][tasks][unit]")
{
    auto fake = std::make_shared<FakeKimaiClient>();

    SessionController controller(fake, KimaiUser{});
    processEvents();
    REQUIRE_FALSE(controller.hasTaskManagement());

    fake->calls.clear();
    controller.loadTasks();
    processEvents();
    REQUIRE_FALSE(fake->calls.contains("requestTasks"));
    REQUIRE(controller.tasksLoad() == SessionController::TasksLoad::NotLoaded);
}

TEST_CASE("SessionController - loads, starts and closes tasks", "[session][tasks][unit]")
{
    auto fake     = std::make_shared<FakeKimaiClient>();
    fake->plugins = taskManagementPlugin();
    fake->tasks   = {openTask(1, "Monthly invoices"), openTask(2, "Review")};

    SessionController controller(fake, KimaiUser{});
    processEvents();
    REQUIRE(controller.hasTaskManagement());

    controller.loadTasks();
    REQUIRE(controller.tasksLoad() == SessionController::TasksLoad::Loading);
    processEvents();
    REQUIRE(controller.tasksLoad() == SessionController::TasksLoad::Ready);
    REQUIRE(controller.tasks()->totalCount() == 2);

    // Start: the task's timer, titled after it
    int started = 0;
    QObject::connect(&controller, &SessionController::taskStarted, [&]
                     { ++started; });
    fake->timeSheetResult = makeTimeSheet(500, QDateTime::currentDateTimeUtc());
    fake->calls.clear();
    controller.startTask(1);
    processEvents();
    REQUIRE(started == 1);
    REQUIRE(fake->lastStartedTaskId == 1);
    REQUIRE(fake->lastUpdatedTimeSheet.description == "Monthly invoices");
    REQUIRE(fake->calls.contains("requestActiveTimeSheets"));

    // Close: leaves the list
    int closed = 0;
    QObject::connect(&controller, &SessionController::taskClosed, [&]
                     { ++closed; });
    controller.closeTask(2);
    processEvents();
    REQUIRE(closed == 1);
    REQUIRE(controller.tasks()->findById(2) == nullptr);
}

TEST_CASE("SessionController - a timer change reloads the loaded tasks", "[session][tasks][unit]")
{
    auto fake     = std::make_shared<FakeKimaiClient>();
    fake->plugins = taskManagementPlugin();
    fake->tasks   = {openTask(1, "Monthly invoices")};

    SessionController controller(fake, KimaiUser{});
    processEvents();
    controller.loadTasks();
    processEvents();

    fake->activeTimeSheets = {makeTimeSheet(5, QDateTime::currentDateTimeUtc())};
    fake->calls.clear();
    controller.syncActiveTimeSheet();
    processEvents();
    REQUIRE(fake->calls.count("requestTasks") == 1);
    REQUIRE(controller.tasksLoad() == SessionController::TasksLoad::Ready); // no skeleton on reload
}

TEST_CASE("SessionController - task failures go to the row, 401 expires the session", "[session][tasks][error][unit]")
{
    auto fake     = std::make_shared<FakeKimaiClient>();
    fake->plugins = taskManagementPlugin();
    fake->tasks   = {openTask(1, "Monthly invoices")};

    SessionController controller(fake, KimaiUser{});
    processEvents();
    controller.loadTasks();
    processEvents();

    SignalProbe probe;
    probe.attach(controller);
    QString                                            action;
    std::optional<SessionController::NotificationKind> failure;
    QObject::connect(&controller,
                     &SessionController::taskActionFailed,
                     [&](int, const QString &a, SessionController::NotificationKind kind, const QString &)
                     {
                         action  = a;
                         failure = kind;
                     });

    fake->failWith("closeTask", ApiErrorKind::Forbidden, 403);
    controller.closeTask(1);
    processEvents();
    REQUIRE(action == "close");
    REQUIRE(failure == SessionController::NotificationKind::Forbidden);
    REQUIRE(controller.tasks()->findById(1) != nullptr); // unchanged
    REQUIRE(probe.notificationCount == 0);

    // A background reload failure keeps the list and warns
    fake->failWith("requestTasks", ApiErrorKind::NoNetwork);
    controller.loadTasks();
    processEvents();
    REQUIRE(controller.tasksLoad() == SessionController::TasksLoad::Ready);
    REQUIRE(probe.notificationCount == 1);
    REQUIRE(probe.lastLevel == SessionController::NotificationLevel::Warning);

    fake->failWith("startTask", ApiErrorKind::AuthFailed, 401);
    controller.startTask(1);
    processEvents();
    REQUIRE(probe.sessionExpiredCount == 1);
}

TEST_CASE("SessionController - a failed first task load shows in the pane", "[session][tasks][error][unit]")
{
    auto fake     = std::make_shared<FakeKimaiClient>();
    fake->plugins = taskManagementPlugin();
    fake->failWith("requestTasks", ApiErrorKind::NoNetwork);

    SessionController controller(fake, KimaiUser{});
    processEvents();

    SignalProbe probe;
    probe.attach(controller);
    controller.loadTasks();
    processEvents();

    REQUIRE(controller.tasksLoad() == SessionController::TasksLoad::Failed);
    REQUIRE(controller.tasksError() == SessionController::NotificationKind::NoNetwork);
    REQUIRE(probe.notificationCount == 0);
}

namespace
{
    KimaiMetaFieldRule requiredField(const QString &name, const QString &type = "text", const QString &rawValue = {})
    {
        return {.name = name, .label = name, .type = type, .rawValue = rawValue, .required = true};
    }

    struct CustomFieldsProbe
    {
        int         saved     = 0;
        int         savedId   = 0;
        int         saveFails = 0;
        int         fails     = 0;
        int         failedId  = 0;
        QStringList pending;

        void attach(SessionController &controller)
        {
            QObject::connect(&controller, &SessionController::timeSheetSaved, [this](int id)
                             {
                                 ++saved;
                                 savedId = id; });
            QObject::connect(&controller, &SessionController::timeSheetSaveFailed, [this](int, SessionController::NotificationKind, const QString &)
                             { ++saveFails; });
            QObject::connect(&controller,
                             &SessionController::customFieldsSaveFailed,
                             [this](int id, SessionController::NotificationKind, const QString &, const QStringList &names)
                             {
                                 ++fails;
                                 failedId = id;
                                 pending  = names;
                             });
        }
    };

    QStringList writtenNames(const FakeKimaiClient &fake)
    {
        QStringList names;
        for (const auto &write : fake.metaWrites)
        {
            names.append(write.name);
        }
        return names;
    }
} // namespace

TEST_CASE("SessionController::needsCustomFields - asks only when something is missing", "[session][customFields][unit]")
{
    auto         fake = std::make_shared<FakeKimaiClient>();
    KimaiProject project;
    project.id          = 7;
    project.customer.id = 3;
    fake->projects      = {project};

    auto restricted       = requiredField("ticket");
    restricted.projectId  = 99;
    auto byCustomer       = requiredField("po");
    byCustomer.customerId = 3;

    SECTION("Nothing applies")
    {
        fake->metaFieldRules = {restricted};
        SessionController controller(fake, KimaiUser{});
        processEvents();
        REQUIRE_FALSE(controller.needsCustomFields(7, 11));
        REQUIRE(controller.needsCustomFields(99, 11));
    }

    SECTION("A field restricted to the project's customer applies")
    {
        fake->metaFieldRules = {byCustomer};
        SessionController controller(fake, KimaiUser{});
        processEvents();
        REQUIRE(controller.needsCustomFields(7, 11));
    }

    SECTION("A default fills it")
    {
        fake->metaFieldRules = {requiredField("kind", "choice", "a|Alpha,b|Beta")};
        SessionController controller(fake, KimaiUser{});
        processEvents();
        REQUIRE_FALSE(controller.needsCustomFields(7, 11));
    }

    SECTION("The restarted entry's value fills it")
    {
        fake->metaFieldRules = {requiredField("ticket")};
        auto source          = makeTimeSheet(40, QDateTime::currentDateTimeUtc().addDays(-1), QDateTime::currentDateTimeUtc().addDays(-1).addSecs(3600));
        source.metaFields    = {
            {"ticket", "KEM-12"}
        };
        fake->timeSheets = {source};
        SessionController controller(fake, KimaiUser{});
        processEvents();
        REQUIRE(controller.needsCustomFields(7, 11));
        REQUIRE_FALSE(controller.needsCustomFields(7, 11, 40));
    }

    SECTION("Only a field Kemai cannot fill is missing")
    {
        fake->metaFieldRules = {requiredField("template", "invoice-template"), requiredField("kind", "choice")};
        SessionController controller(fake, KimaiUser{});
        processEvents();
        REQUIRE_FALSE(controller.needsCustomFields(7, 11));
    }

    SECTION("An invalid value is asked for again")
    {
        fake->metaFieldRules = {requiredField("contact", "email")};
        auto source          = makeTimeSheet(40, QDateTime::currentDateTimeUtc().addDays(-1), QDateTime::currentDateTimeUtc().addDays(-1).addSecs(3600));
        source.metaFields    = {
            {"contact", "john@"}
        };
        fake->timeSheets = {source};
        SessionController controller(fake, KimaiUser{});
        processEvents();
        REQUIRE(controller.needsCustomFields(7, 11, 40));
    }
}

TEST_CASE("SessionController::startTimeSheet - writes defaulted and reused values", "[session][customFields][unit]")
{
    auto fake            = std::make_shared<FakeKimaiClient>();
    fake->metaFieldRules = {requiredField("kind", "choice", "a|Alpha,b|Beta"), requiredField("ticket"), requiredField("template", "invoice-template")};
    auto source          = makeTimeSheet(40, QDateTime::currentDateTimeUtc().addDays(-1), QDateTime::currentDateTimeUtc().addDays(-1).addSecs(3600));
    source.metaFields    = {
        {"kind",   "b"     },
        {"ticket", "KEM-12"}
    };
    fake->timeSheets      = {source};
    fake->timeSheetResult = makeTimeSheet(500, QDateTime::currentDateTimeUtc());

    SessionController controller(fake, KimaiUser{});
    processEvents();

    SECTION("Restart reuses the entry's values")
    {
        controller.startTimeSheet(11, 7, 40);
        processEvents();
        REQUIRE(fake->metaWrites.size() == 2);
        REQUIRE(fake->metaWrites[0].timeSheetId == 500);
        REQUIRE(fake->metaWrites[0].name == "kind");
        REQUIRE(fake->metaWrites[0].value == "b");
        REQUIRE(fake->metaWrites[1].value == "KEM-12");
    }

    SECTION("A plain start writes the defaults only")
    {
        controller.startTimeSheet(11, 7);
        processEvents();
        REQUIRE(writtenNames(*fake) == QStringList{"kind"});
        REQUIRE(fake->metaWrites[0].value == "a");
    }

    SECTION("A value Kimai already has is not written again")
    {
        fake->timeSheetResult.metaFields = {
            {"kind", "a"}
        };
        controller.startTimeSheet(11, 7);
        processEvents();
        REQUIRE(fake->metaWrites.isEmpty());
    }

    SECTION("A failed write is reported for the running entry")
    {
        CustomFieldsProbe probe;
        probe.attach(controller);
        fake->failWith("setTimeSheetMeta:kind", ApiErrorKind::NoNetwork);
        controller.startTimeSheet(11, 7, 40);
        processEvents();
        REQUIRE(probe.fails == 1);
        REQUIRE(probe.failedId == 500);
        REQUIRE(probe.pending == QStringList{"kind", "ticket"});
    }
}

TEST_CASE("SessionController::startTimeSheetWith - starts, then writes the values one by one", "[session][customFields][unit]")
{
    auto fake             = std::make_shared<FakeKimaiClient>();
    fake->metaFieldRules  = {requiredField("ticket"), requiredField("contact", "email"), requiredField("other")};
    fake->timeSheetResult = makeTimeSheet(500, QDateTime::currentDateTimeUtc());
    auto running          = fake->timeSheetResult;

    SessionController controller(fake, KimaiUser{});
    processEvents();
    CustomFieldsProbe probe;
    probe.attach(controller);

    const QVariantMap values{
        {"ticket",  "KEM-12"          },
        {"contact", "john@example.com"},
        {"other",   "x"               },
        {"removed", "not applying"    }
    };

    SECTION("All written")
    {
        controller.startTimeSheetWith(7, 11, "Release", {"kemai"}, {}, values);
        processEvents();
        REQUIRE(writtenNames(*fake) == QStringList{"ticket", "contact", "other"});
        REQUIRE(probe.saved == 1);
        REQUIRE(probe.savedId == 500);
    }

    SECTION("Nothing starts when the start fails")
    {
        fake->failWith("startTimeSheet", ApiErrorKind::NoNetwork);
        controller.startTimeSheetWith(7, 11, {}, {}, {}, values);
        processEvents();
        REQUIRE(probe.saveFails == 1);
        REQUIRE(fake->metaWrites.isEmpty());
    }

    SECTION("A failure on the second keeps the first; a retry sends only the pending ones")
    {
        fake->timeSheets = {running};
        fake->failWith("setTimeSheetMeta:contact", ApiErrorKind::NoNetwork);
        controller.startTimeSheetWith(7, 11, {}, {}, {}, values);
        processEvents();
        REQUIRE(writtenNames(*fake) == QStringList{"ticket", "contact"});
        REQUIRE(probe.saved == 0);
        REQUIRE(probe.fails == 1);
        REQUIRE(probe.pending == QStringList{"contact", "other"});

        // Retry from the dialog, now in edit mode on the running entry
        fake->errors.clear();
        fake->metaWrites.clear();
        REQUIRE(controller.updateTimeSheet(500, 7, 11, {}, {}, {}, {}, {}, values));
        processEvents();
        REQUIRE(writtenNames(*fake) == QStringList{"contact", "other"});
        REQUIRE(probe.saved == 1);
    }

    SECTION("A retry of the values alone writes only the pending ones and keeps the rest of the entry")
    {
        fake->timeSheets = {running};
        fake->failWith("setTimeSheetMeta:contact", ApiErrorKind::NoNetwork);
        controller.startTimeSheetWith(7, 11, {}, {}, {}, values);
        processEvents();
        REQUIRE(probe.fails == 1);

        fake->errors.clear();
        fake->metaWrites.clear();
        fake->calls.clear();
        controller.saveCustomValues(500, 7, 11, values);
        processEvents();
        REQUIRE(writtenNames(*fake) == QStringList{"contact", "other"});
        REQUIRE_FALSE(fake->calls.contains("updateTimeSheet"));
        REQUIRE(probe.saved == 1);
        REQUIRE(probe.savedId == 500);
    }

    SECTION("401 while writing expires the session")
    {
        SignalProbe expiry;
        expiry.attach(controller);
        fake->failWith("setTimeSheetMeta:ticket", ApiErrorKind::AuthFailed, 401);
        controller.startTimeSheetWith(7, 11, {}, {}, {}, values);
        processEvents();
        REQUIRE(expiry.sessionExpiredCount == 1);
        REQUIRE(probe.fails == 0);
    }

    SECTION("A field removed from Kimai meanwhile counts as written")
    {
        fake->failWith("setTimeSheetMeta:ticket", ApiErrorKind::NotFound, 404);
        controller.startTimeSheetWith(7, 11, {}, {}, {}, values);
        processEvents();
        REQUIRE(writtenNames(*fake) == QStringList{"ticket", "contact", "other"});
        REQUIRE(probe.saved == 1);
    }
}

TEST_CASE("SessionController::updateTimeSheet - writes only the values that changed", "[session][customFields][unit]")
{
    auto fake            = std::make_shared<FakeKimaiClient>();
    fake->metaFieldRules = {requiredField("ticket"), requiredField("contact", "email")};
    auto entry           = makeTimeSheet(40, QDateTime::currentDateTimeUtc().addSecs(-7200), QDateTime::currentDateTimeUtc().addSecs(-3600));
    entry.metaFields     = {
        {"ticket",  "KEM-12"          },
        {"contact", "john@example.com"}
    };
    fake->timeSheets = {entry};

    SessionController controller(fake, KimaiUser{});
    processEvents();
    CustomFieldsProbe probe;
    probe.attach(controller);

    REQUIRE(controller.updateTimeSheet(40, 7, 11, "Edited", {
    },
                                       {},
                                       {},
                                       {},
                                       {{"ticket", "KEM-12"}, {"contact", "jane@example.com"}}));
    processEvents();
    REQUIRE(writtenNames(*fake) == QStringList{"contact"});
    REQUIRE(probe.saved == 1);

    fake->metaWrites.clear();
    REQUIRE(controller.updateTimeSheet(40, 7, 11, "Edited again", {
    },
                                       {},
                                       {},
                                       {},
                                       {{"ticket", "KEM-12"}, {"contact", "jane@example.com"}}));
    processEvents();
    REQUIRE(fake->metaWrites.isEmpty());
    REQUIRE(probe.saved == 2);
}

TEST_CASE("SessionController - task starts write the custom values", "[session][customFields][tasks][unit]")
{
    auto fake             = std::make_shared<FakeKimaiClient>();
    fake->plugins         = taskManagementPlugin();
    fake->tasks           = {openTask(1, "Monthly invoices")};
    fake->metaFieldRules  = {requiredField("kind", "choice", "a|Alpha,b|Beta"), requiredField("ticket")};
    fake->timeSheetResult = makeTimeSheet(500, QDateTime::currentDateTimeUtc());

    SessionController controller(fake, KimaiUser{});
    processEvents();
    controller.loadTasks();
    processEvents();
    CustomFieldsProbe probe;
    probe.attach(controller);

    SECTION("Plain start: the defaults")
    {
        controller.startTask(1);
        processEvents();
        REQUIRE(writtenNames(*fake) == QStringList{"kind"});
    }

    SECTION("From the dialog: the values given, then saved")
    {
        controller.startTaskWith(1, {
                                        {"kind",   "b"     },
                                        {"ticket", "KEM-12"}
        });
        processEvents();
        REQUIRE(writtenNames(*fake) == QStringList{"kind", "ticket"});
        REQUIRE(fake->metaWrites[0].value == "b");
        REQUIRE(probe.saved == 1);
        REQUIRE(probe.savedId == 500);
    }

    SECTION("From the dialog: a refused start goes back to it")
    {
        fake->failWith("startTask", ApiErrorKind::NoNetwork);
        controller.startTaskWith(1, {
                                        {"ticket", "KEM-12"}
        });
        processEvents();
        REQUIRE(probe.saveFails == 1);
        REQUIRE(fake->metaWrites.isEmpty());
    }
}
