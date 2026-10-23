// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// Qt headers
#include <QDate>
#include <QDateTime>
#include <QString>

namespace kemai
{
    class FormatHelpers
    {
    public:
        // Returns a full HH:MM:SS string, e.g. "01:23:45". Suitable for live timers.
        static QString formatDuration(qint64 totalSeconds);

        // Returns a compact "Xh YYm" string, e.g. "1h 23m". Suitable for timesheet lists.
        static QString formatDurationShort(qint64 totalSeconds);

        // Returns "HH:mm – HH:mm" in local time, prefixed by the day when it is not `today`
        // ("Yesterday 16:00 – 18:10", "Mon, Sep 15 16:00 – 18:10"). An invalid endAt reads
        // "now"; an end on another day than the begin gets its own day ("… – Today 01:30").
        static QString formatTimeRange(const QDateTime &beginAt, const QDateTime &endAt, const QDate &today = QDate::currentDate());
    };
} // namespace kemai
