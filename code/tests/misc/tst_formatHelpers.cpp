// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include <catch2/catch_test_macros.hpp>

#include <QLocale>

#include <misc/formatHelpers.h>

using namespace kemai;

namespace
{
    // Fixed "today" and local-time datetimes: results do not depend on the machine clock or timezone
    const QDate gToday(2026, 9, 15); // Tuesday

    QDateTime local(const QDate &date, int hour, int minute)
    {
        return QDateTime(date, QTime(hour, minute));
    }

    struct CLocaleScope
    {
        QLocale previous;
        CLocaleScope()
        { QLocale::setDefault(QLocale::c()); }
        ~CLocaleScope()
        { QLocale::setDefault(previous); }
    };
} // namespace

TEST_CASE("FormatHelpers::formatTimeRange - today has no day prefix", "[format][unit]")
{
    REQUIRE(FormatHelpers::formatTimeRange(local(gToday, 14, 25), local(gToday, 17, 0), gToday) == QStringLiteral("14:25 – 17:00"));
    REQUIRE(FormatHelpers::formatTimeRange(local(gToday, 14, 25), {}, gToday) == QStringLiteral("14:25 – now"));
}

TEST_CASE("FormatHelpers::formatTimeRange - yesterday is named", "[format][unit]")
{
    const auto yesterday = gToday.addDays(-1);

    REQUIRE(FormatHelpers::formatTimeRange(local(yesterday, 16, 0), local(yesterday, 18, 10), gToday) == QStringLiteral("Yesterday 16:00 – 18:10"));
    REQUIRE(FormatHelpers::formatTimeRange(local(yesterday, 16, 0), {}, gToday) == QStringLiteral("Yesterday 16:00 – now"));
}

TEST_CASE("FormatHelpers::formatTimeRange - older days show the date", "[format][unit]")
{
    CLocaleScope cLocale;
    const auto   lastWeek = gToday.addDays(-8); // Mon, Sep 7

    REQUIRE(FormatHelpers::formatTimeRange(local(lastWeek, 9, 5), local(lastWeek, 10, 0), gToday) == QStringLiteral("Mon, Sep 7 09:05 – 10:00"));
}

TEST_CASE("FormatHelpers::formatTimeRange - an end on another day is named explicitly", "[format][unit]")
{
    const auto yesterday = gToday.addDays(-1);

    REQUIRE(FormatHelpers::formatTimeRange(local(yesterday, 23, 0), local(gToday, 1, 30), gToday) == QStringLiteral("Yesterday 23:00 – Today 01:30"));
}
