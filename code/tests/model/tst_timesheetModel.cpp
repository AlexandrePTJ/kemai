// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include <catch2/catch_test_macros.hpp>

#include <QJsonDocument>
#include <QJsonObject>

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

TEST_CASE("TimesheetModel - custom values are read from replies and exposed, never sent back", "[model][unit][customFields]")
{
    const auto json = QJsonDocument::fromJson(R"({"id": 7, "begin": "2026-10-27T09:00:00+0100", "end": "2026-10-27T10:00:00+0100",
        "project": 1, "activity": 2, "description": "", "tags": [],
        "metaFields": [{"name": "ticket", "value": "KEM-42"}, {"name": "billable", "value": "1"}, {"name": "note", "value": null}]})")
                          .object();

    const auto ts = KimaiTimeSheet::fromJson(json);
    REQUIRE(ts.metaFields.size() == 3);
    CHECK(ts.metaFields.value("ticket") == "KEM-42");
    CHECK(ts.metaFields.value("billable") == "1");
    CHECK(ts.metaFields.value("note").isEmpty());

    // Kimai's timesheet form refuses metaFields: the entry body never carries them
    CHECK_FALSE(ts.toJson(KimaiTimeSheetConfig::TrackingMode::Default).contains("metaFields"));

    TimesheetModel model;
    model.setTimeSheets({ts});
    const auto role = static_cast<int>(TimesheetModel::TimesheetRole::CustomValues);
    REQUIRE(model.roleNames().value(role) == "customValues");
    const auto values = model.data(model.index(0), role).toMap();
    CHECK(values.value("ticket").toString() == "KEM-42");
    CHECK(values.size() == 3);
}

TEST_CASE("TimesheetModel::entry - an entry's roles by name", "[model][unit]")
{
    KimaiTimeSheet ts;
    ts.id         = 5;
    ts.beginAt    = QDateTime::currentDateTimeUtc().addSecs(-3600);
    ts.metaFields = {
        {"ticket", "KEM-1"}
    };

    TimesheetModel model;
    model.setTimeSheets({ts});

    const auto entry = model.entry(5);
    REQUIRE(entry.value("timesheetId").toInt() == 5);
    REQUIRE(entry.value("status").toString() == "active");
    REQUIRE(entry.value("customValues").toMap().value("ticket") == "KEM-1");
    REQUIRE(model.entry(6).isEmpty());
}
