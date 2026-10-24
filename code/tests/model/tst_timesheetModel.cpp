// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include <catch2/catch_test_macros.hpp>

#include <model/timesheetModel.h>

using namespace kemai;

TEST_CASE("TimesheetModel - tags role exposes the timesheet tags", "[model][unit]")
{
    KimaiTimeSheet ts;
    ts.id      = 1;
    ts.beginAt = QDateTime::currentDateTimeUtc().addSecs(-3600);
    ts.endAt   = QDateTime::currentDateTimeUtc();
    ts.tags    = {"QML", "Frontend"};

    TimesheetModel model;
    model.setTimeSheets({ts});

    const auto role = static_cast<int>(TimesheetModel::TimesheetRole::Tags);
    REQUIRE(model.roleNames().value(role) == "tags");
    REQUIRE(model.data(model.index(0), role).toStringList() == QStringList{"QML", "Frontend"});
}

TEST_CASE("TimesheetModel - running entry comes first, then most recent", "[model][unit]")
{
    const auto now = QDateTime::currentDateTimeUtc();

    KimaiTimeSheet past;
    past.id      = 1;
    past.beginAt = now.addDays(-1);
    past.endAt   = now.addDays(-1).addSecs(3600);

    KimaiTimeSheet future;
    future.id      = 2;
    future.beginAt = now.addDays(7);
    future.endAt   = now.addDays(7).addSecs(3600);

    KimaiTimeSheet running;
    running.id      = 3;
    running.beginAt = now.addSecs(-600);

    TimesheetModel model;
    model.setTimeSheets({past, running, future});

    const auto role = static_cast<int>(TimesheetModel::TimesheetRole::TimesheetId);
    REQUIRE(model.data(model.index(0), role).toInt() == 3);
    REQUIRE(model.data(model.index(1), role).toInt() == 2);
    REQUIRE(model.data(model.index(2), role).toInt() == 1);
}
