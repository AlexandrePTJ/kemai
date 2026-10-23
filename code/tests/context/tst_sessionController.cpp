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

TEST_CASE("SessionController::updateTimeSheetTimes - rejects end before begin", "[session][unit]")
{
    const auto begin = QDateTime::fromString("2026-06-10T10:00:00Z", Qt::ISODate);
    const auto end   = QDateTime::fromString("2026-06-10T12:00:00Z", Qt::ISODate);

    auto fake = std::make_shared<FakeKimaiClient>();
    fake->timeSheets.append(makeTimeSheet(42, begin, end));

    SessionController controller(fake, KimaiUser{});
    processEvents();

    fake->calls.clear();
    controller.updateTimeSheetTimes(42, {}, begin.addSecs(-60));
    processEvents();

    REQUIRE_FALSE(fake->calls.contains("updateTimeSheet"));
}

TEST_CASE("SessionController::updateTimeSheetTimes - invalid datetime leaves field unchanged", "[session][unit]")
{
    const auto begin  = QDateTime::fromString("2026-06-10T10:00:00Z", Qt::ISODate);
    const auto end    = QDateTime::fromString("2026-06-10T12:00:00Z", Qt::ISODate);
    const auto newEnd = QDateTime::fromString("2026-06-10T13:30:00Z", Qt::ISODate);

    auto fake = std::make_shared<FakeKimaiClient>();
    fake->timeSheets.append(makeTimeSheet(42, begin, end));

    SessionController controller(fake, KimaiUser{});
    processEvents();

    controller.updateTimeSheetTimes(42, {}, newEnd);
    processEvents();

    REQUIRE(fake->calls.contains("updateTimeSheet"));
    REQUIRE(fake->lastUpdatedTimeSheet.id == 42);
    REQUIRE(fake->lastUpdatedTimeSheet.beginAt == begin);
    REQUIRE(fake->lastUpdatedTimeSheet.endAt == newEnd.toUTC());
}
