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
