// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include <catch2/catch_test_macros.hpp>
#include <spdlog/spdlog.h>

#include <QCoreApplication>

#include <misc/logJournalSink.h>
#include <model/logEntryModel.h>

using namespace kemai;

namespace
{
    LogEntryModel::Entry entryAt(const QString &time, LogEntryModel::Level level, const QString &message)
    {
        return {.time = QDateTime(QDate(2026, 10, 25), QTime::fromString(time, "HH:mm:ss")), .level = level, .message = message};
    }
} // namespace

TEST_CASE("LogEntryModel - keeps the last entries and counts errors and warnings", "[logJournal][unit]")
{
    LogEntryModel model(nullptr, 3);
    model.append(entryAt("10:00:00", LogEntryModel::Level::Error, "first"));
    model.append(entryAt("10:00:01", LogEntryModel::Level::Warning, "second"));
    model.append(entryAt("10:00:02", LogEntryModel::Level::Info, "third"));
    REQUIRE(model.rowCount() == 3);
    REQUIRE(model.errorCount() == 1);
    REQUIRE(model.warningCount() == 1);

    model.append(entryAt("10:00:03", LogEntryModel::Level::Debug, "fourth"));
    REQUIRE(model.rowCount() == 3);
    REQUIRE(model.entry(0).message == "second");
    REQUIRE(model.entry(2).message == "fourth");
    REQUIRE(model.errorCount() == 0);
    REQUIRE(model.warningCount() == 1);

    const auto index = model.index(2);
    REQUIRE(model.data(index, static_cast<int>(LogEntryModel::Role::Time)).toString() == "10:00:03");
    REQUIRE(model.data(index, static_cast<int>(LogEntryModel::Role::Level)).toInt() == static_cast<int>(LogEntryModel::Level::Debug));
}

TEST_CASE("LogEntryModel::textLine - time, padded level and full message", "[logJournal][unit]")
{
    const auto line = LogEntryModel::textLine(entryAt("14:48:21", LogEntryModel::Level::Error, "PATCH timesheets/4179 → 403"));
    REQUIRE(line == "14:48:21 ERROR   PATCH timesheets/4179 → 403");
    REQUIRE(LogEntryModel::textLine(entryAt("14:48:21", LogEntryModel::Level::Warning, "w")) == "14:48:21 WARNING w");
}

TEST_CASE("LogJournalSink - entries reach the model in order, not after detach", "[logJournal][unit]")
{
    LogEntryModel  model;
    auto           sink = std::make_shared<LogJournalSink>(&model);
    spdlog::logger logger("journal-test", sink);
    logger.set_level(spdlog::level::debug);

    logger.debug("one");
    logger.warn("two");
    logger.critical("three");
    // Queued: nothing until the event loop runs
    REQUIRE(model.rowCount() == 0);
    QCoreApplication::processEvents();

    REQUIRE(model.rowCount() == 3);
    REQUIRE(model.entry(0).level == LogEntryModel::Level::Debug);
    REQUIRE(model.entry(1).message == "two");
    REQUIRE(model.entry(1).level == LogEntryModel::Level::Warning);
    REQUIRE(model.entry(2).level == LogEntryModel::Level::Error);

    sink->detach();
    logger.info("after");
    QCoreApplication::processEvents();
    REQUIRE(model.rowCount() == 3);
}
