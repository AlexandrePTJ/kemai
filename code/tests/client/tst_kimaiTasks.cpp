// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include <catch2/catch_test_macros.hpp>

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>

#include <client/kimaiSystem.h>
#include <client/kimaiTimesheets.h>

using namespace kemai;

// Replies recorded on demo.kimai.org (Kimai 2.70.2, TaskManagement plugin)
namespace
{
    QJsonDocument readReply(const QString &name)
    {
        QFile file(QStringLiteral(KEMAI_TEST_DATA_DIR "/kimai-tasks/%1.json").arg(name));
        REQUIRE(file.open(QIODevice::ReadOnly));
        return QJsonDocument::fromJson(file.readAll());
    }
} // namespace

TEST_CASE("KimaiTask::fromJson - open tasks reply", "[tasks][unit]")
{
    const auto array = readReply("tasks").array();
    REQUIRE(array.size() == 4);

    QList<KimaiTask> tasks;
    for (const auto &value : array)
    {
        tasks.append(KimaiTask::fromJson(value));
    }

    const auto &inProgress = tasks[0];
    REQUIRE(inProgress.id == 42);
    REQUIRE(inProgress.status == KimaiTask::Status::Progress);
    REQUIRE_FALSE(inProgress.title.isEmpty());
    REQUIRE(inProgress.activity.id > 0);
    REQUIRE(inProgress.project.id > 0);
    REQUIRE(inProgress.endAt.isValid());
    REQUIRE(inProgress.estimation == 20334);

    REQUIRE(tasks[1].status == KimaiTask::Status::Pending);
    REQUIRE(tasks[1].estimation == 0); // none

    REQUIRE_FALSE(tasks[2].endAt.isValid()); // no due date
}

TEST_CASE("KimaiTask - start answers with the started timesheet", "[tasks][unit]")
{
    const auto timeSheet = KimaiTimeSheet::fromJson(readReply("start").object());
    REQUIRE(timeSheet.id == 11869);
    REQUIRE(timeSheet.beginAt.isValid());
    REQUIRE_FALSE(timeSheet.endAt.isValid());
}

TEST_CASE("KimaiTask - close answers with the closed task", "[tasks][unit]")
{
    const auto task = KimaiTask::fromJson(readReply("close").object());
    REQUIRE(task.id == 47);
    REQUIRE(task.status == KimaiTask::Status::Closed);
}

TEST_CASE("KimaiPlugin - the plugin list names TaskManagement", "[tasks][unit]")
{
    const auto array   = readReply("plugins").array();
    const auto plugins = [&]
    {
        QList<KimaiPlugin> list;
        for (const auto &value : array)
        {
            list.append(KimaiPlugin::fromJson(value));
        }
        return list;
    }();
    REQUIRE(std::ranges::any_of(plugins, [](const KimaiPlugin &p)
                                { return p.apiPlugin == ApiPlugin::TaskManagement; }));
}
