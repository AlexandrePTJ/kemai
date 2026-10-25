// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include <catch2/catch_test_macros.hpp>

#include <model/taskModel.h>

using namespace kemai;

namespace
{
    const QDate Today(2026, 10, 24);

    KimaiTask makeTask(int id, const QString &title, KimaiTask::Status status, const QDateTime &due = {}, int activeTimeSheetId = 0)
    {
        KimaiTask task;
        task.id            = id;
        task.title         = title;
        task.status        = status;
        task.endAt         = due;
        task.project.name  = "Website";
        task.activity.name = "Design";
        if (activeTimeSheetId > 0)
        {
            KimaiTimeSheet ts;
            ts.id = activeTimeSheetId;
            task.activeTimeSheets.append(ts);
        }
        return task;
    }

    QDateTime day(int offset)
    {
        return QDateTime(Today.addDays(offset), QTime(12, 0));
    }

    QStringList column(const TaskModel &model, TaskModel::Role role)
    {
        QStringList values;
        for (int row = 0; row < model.rowCount(); ++row)
        {
            values.append(model.data(model.index(row), static_cast<int>(role)).toString());
        }
        return values;
    }
} // namespace

TEST_CASE("TaskModel - groups and order", "[tasks][model][unit]")
{
    TaskModel model;
    model.setToday(Today);
    model.setTasks({
                       makeTask(1, "No due B", KimaiTask::Status::Pending),
                       makeTask(2, "Upcoming far", KimaiTask::Status::Pending, day(10)),
                       makeTask(3, "Overdue recent", KimaiTask::Status::Pending, day(-1)),
                       makeTask(4, "In progress", KimaiTask::Status::Progress, day(-5)),
                       makeTask(5, "Due today", KimaiTask::Status::Pending, day(0)),
                       makeTask(6, "Overdue old", KimaiTask::Status::Pending, day(-9)),
                       makeTask(7, "Running", KimaiTask::Status::Pending, day(3), 900),
                       makeTask(8, "No due A", KimaiTask::Status::Pending),
                   },
                   900);

    REQUIRE(column(model, TaskModel::Role::Title) == QStringList{"Running", "In progress", "Overdue old", "Overdue recent", "Due today", "Upcoming far", "No due A", "No due B"});
    REQUIRE(column(model, TaskModel::Role::Group) == QStringList{"progress", "progress", "overdue", "overdue", "upcoming", "upcoming", "none", "none"});
    REQUIRE(model.data(model.index(4), static_cast<int>(TaskModel::Role::DueState)) == "today");
    REQUIRE(model.data(model.index(2), static_cast<int>(TaskModel::Role::OverdueDays)) == 9);
    REQUIRE(model.data(model.index(0), static_cast<int>(TaskModel::Role::Running)).toBool());
    REQUIRE(model.runningTaskTitle() == "Running");
    REQUIRE(model.groupCount("overdue") == 2);

    // The timer stops: no running task any more
    model.setActiveTimeSheetId(0);
    REQUIRE(model.runningTaskTitle().isEmpty());
    REQUIRE(column(model, TaskModel::Role::Title).first() == "In progress");
}

TEST_CASE("TaskModel - filter on title, project, activity and todo", "[tasks][model][unit]")
{
    TaskModel model;
    model.setToday(Today);
    auto withTodo = makeTask(2, "Invoices", KimaiTask::Status::Pending);
    withTodo.todo = "Send the monthly REPORT";
    model.setTasks({makeTask(1, "Monthly invoices", KimaiTask::Status::Pending, day(2)), withTodo, makeTask(3, "Other", KimaiTask::Status::Progress)}, 0);

    model.setFilterText("invoice");
    REQUIRE(model.rowCount() == 2);
    REQUIRE(model.totalCount() == 3);
    REQUIRE(column(model, TaskModel::Role::Group) == QStringList{"matches", "matches"});
    REQUIRE(model.groupCount("matches") == 2);

    model.setFilterText("report");
    REQUIRE(column(model, TaskModel::Role::Title) == QStringList{"Invoices"});

    model.setFilterText("design"); // activity
    REQUIRE(model.rowCount() == 3);

    model.setFilterText("nothing like it");
    REQUIRE(model.rowCount() == 0);

    model.setFilterText("");
    model.removeTask(3);
    REQUIRE(model.totalCount() == 2);
}
