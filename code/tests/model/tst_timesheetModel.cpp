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

TEST_CASE("TimesheetModel - totals count the week's entries, not only the listed ones", "[model][unit]")
{
    const auto todayStart = QDate::currentDate().startOfDay();

    KimaiTimeSheet listed;
    listed.id      = 1;
    listed.beginAt = todayStart;
    listed.endAt   = todayStart.addSecs(3600);

    KimaiTimeSheet older = listed;
    older.id             = 2;
    older.beginAt        = todayStart.addSecs(7200);
    older.endAt          = todayStart.addSecs(9000);

    TimesheetModel model;
    model.setTimeSheets({listed});
    model.setTotalsTimeSheets({listed, older});

    REQUIRE(model.rowCount() == 1);
    REQUIRE(model.todayDuration() == "1h 30m");
    REQUIRE(model.weekDuration() == "1h 30m");
}

TEST_CASE("TimesheetModel::totalsPeriodStart - covers the week from the day before Monday", "[model][unit]")
{
    const auto start = TimesheetModel::totalsPeriodStart();

    REQUIRE(start.time() == QTime(0, 0));
    REQUIRE(start.date().dayOfWeek() == Qt::Sunday);
    REQUIRE(start.date().daysTo(QDate::currentDate()) >= 1);
    REQUIRE(start.date().daysTo(QDate::currentDate()) <= 7);
}

namespace
{
    KimaiTimeSheet entryWith(int id, const QString &project, const QString &activity, const QString &description, const QStringList &tags = {})
    {
        const auto todayStart = QDate::currentDate().startOfDay();

        KimaiTimeSheet ts;
        ts.id            = id;
        ts.project.name  = project;
        ts.activity.name = activity;
        ts.description   = description;
        ts.tags          = tags;
        ts.beginAt       = todayStart.addSecs(id * 60);
        ts.endAt         = todayStart.addSecs(id * 60 + 30);
        return ts;
    }

    QList<int> shownIds(const TimesheetModel &model)
    {
        QList<int> ids;
        for (int row = 0; row < model.rowCount(); ++row)
        {
            ids.append(model.data(model.index(row), static_cast<int>(TimesheetModel::TimesheetRole::TimesheetId)).toInt());
        }
        std::ranges::sort(ids);
        return ids;
    }
} // namespace

TEST_CASE("TimesheetModel - the filter matches project, activity, description and tags", "[model][filter][unit]")
{
    TimesheetModel model;
    model.setTimeSheets({entryWith(1, "Helios CMS", "Code review", "PR 42"), entryWith(2, "Atlas", "Design", "Mockups", {"Frontend"}), entryWith(3, "Atlas", "Meeting", "Weekly review"), entryWith(4, "Rocket", "Support", "Ticket")});

    model.setFilterText("review");
    REQUIRE(shownIds(model) == QList<int>{1, 3});
    REQUIRE(model.totalCount() == 4);

    model.setFilterText("FRONT");
    REQUIRE(shownIds(model) == QList<int>{2});

    model.setFilterText("atlas");
    REQUIRE(shownIds(model) == QList<int>{2, 3});

    model.setFilterText("  ");
    REQUIRE(model.rowCount() == 4);
}

TEST_CASE("TimesheetModel - the filter text survives a reload", "[model][filter][unit]")
{
    TimesheetModel model;
    model.setFilterText("review");
    model.setTimeSheets({entryWith(1, "Helios CMS", "Code review", ""), entryWith(2, "Atlas", "Design", "")});

    REQUIRE(model.filterText() == "review");
    REQUIRE(shownIds(model) == QList<int>{1});

    // An entry the filter hides is still found by id (edit dialog)
    REQUIRE(model.entry(2).value("activityName") == "Design");
}

TEST_CASE("TimesheetModel - a running entry started before the period is set apart", "[model][unit]")
{
    const auto today       = QDate::currentDate();
    const auto periodStart = today.addDays(-3);
    const auto sectionRole = static_cast<int>(TimesheetModel::TimesheetRole::Section);

    KimaiTimeSheet running;
    running.id      = 9;
    running.beginAt = QDateTime(periodStart.addDays(-2), QTime(16, 0));

    KimaiTimeSheet listed;
    listed.id      = 1;
    listed.beginAt = today.startOfDay();
    listed.endAt   = today.startOfDay().addSecs(600);

    TimesheetModel model;
    model.setTimeSheets({listed, running}, periodStart);
    REQUIRE(model.data(model.index(0), sectionRole) == "before");
    REQUIRE(model.data(model.index(1), sectionRole) == "period");

    // Started within the period: no section
    running.beginAt = periodStart.startOfDay().addSecs(3600);
    model.setTimeSheets({listed, running}, periodStart);
    REQUIRE(model.data(model.index(0), sectionRole).toString().isEmpty());
    REQUIRE(model.data(model.index(1), sectionRole).toString().isEmpty());
}

TEST_CASE("TimesheetModel - entries starting after now are planned", "[model][unit]")
{
    const auto now         = QDateTime::currentDateTimeUtc();
    const auto plannedRole = static_cast<int>(TimesheetModel::TimesheetRole::Planned);

    KimaiTimeSheet past;
    past.id      = 1;
    past.beginAt = now.addSecs(-3600);
    past.endAt   = now.addSecs(-1800);

    KimaiTimeSheet future;
    future.id      = 2;
    future.beginAt = now.addDays(1);
    future.endAt   = now.addDays(1).addSecs(3600);

    TimesheetModel model;
    model.setTimeSheets({past, future});
    REQUIRE(model.data(model.index(0), plannedRole).toBool());
    REQUIRE_FALSE(model.data(model.index(1), plannedRole).toBool());
}
