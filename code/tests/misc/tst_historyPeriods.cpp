// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include <catch2/catch_test_macros.hpp>

#include <misc/historyPeriods.h>

using namespace kemai;

TEST_CASE("HistoryPeriods::range - this week runs Monday to Sunday", "[historyPeriods][unit]")
{
    // Thursday October 29, 2026
    REQUIRE(HistoryPeriods::range(HistoryPeriod::ThisWeek, QDate(2026, 10, 29)) == HistoryRange{QDate(2026, 10, 26), QDate(2026, 11, 1)});
    // Monday and Sunday belong to their own week
    REQUIRE(HistoryPeriods::range(HistoryPeriod::ThisWeek, QDate(2026, 10, 26)).first == QDate(2026, 10, 26));
    REQUIRE(HistoryPeriods::range(HistoryPeriod::ThisWeek, QDate(2026, 11, 1)).first == QDate(2026, 10, 26));
}

TEST_CASE("HistoryPeriods::range - week across a year change", "[historyPeriods][unit]")
{
    // Thursday December 31, 2026
    REQUIRE(HistoryPeriods::range(HistoryPeriod::ThisWeek, QDate(2026, 12, 31)) == HistoryRange{QDate(2026, 12, 28), QDate(2027, 1, 3)});
}

TEST_CASE("HistoryPeriods::range - this month covers the whole month", "[historyPeriods][unit]")
{
    REQUIRE(HistoryPeriods::range(HistoryPeriod::ThisMonth, QDate(2026, 10, 29)) == HistoryRange{QDate(2026, 10, 1), QDate(2026, 10, 31)});
    REQUIRE(HistoryPeriods::range(HistoryPeriod::ThisMonth, QDate(2028, 2, 10)) == HistoryRange{QDate(2028, 2, 1), QDate(2028, 2, 29)});
    REQUIRE(HistoryPeriods::range(HistoryPeriod::ThisMonth, QDate(2026, 12, 1)) == HistoryRange{QDate(2026, 12, 1), QDate(2026, 12, 31)});
}

TEST_CASE("HistoryPeriods::range - last 30 days end today", "[historyPeriods][unit]")
{
    REQUIRE(HistoryPeriods::range(HistoryPeriod::Last30Days, QDate(2026, 10, 29)) == HistoryRange{QDate(2026, 9, 30), QDate(2026, 10, 29)});
    // Across a year change: 30 calendar days
    REQUIRE(HistoryPeriods::range(HistoryPeriod::Last30Days, QDate(2027, 1, 10)).first == QDate(2026, 12, 12));
}

TEST_CASE("HistoryPeriods - keys round trip, unknown is this week", "[historyPeriods][unit]")
{
    for (const auto period : {HistoryPeriod::ThisWeek, HistoryPeriod::ThisMonth, HistoryPeriod::Last30Days})
    {
        REQUIRE(HistoryPeriods::fromKey(HistoryPeriods::keyOf(period)) == period);
    }
    REQUIRE(HistoryPeriods::fromKey("year") == HistoryPeriod::ThisWeek);
    REQUIRE(HistoryPeriods::fromKey({}) == HistoryPeriod::ThisWeek);
}
