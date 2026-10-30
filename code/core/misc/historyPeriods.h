// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// Qt headers
#include <QDate>
#include <QString>

namespace kemai
{
    enum class HistoryPeriod
    {
        ThisWeek,
        ThisMonth,
        Last30Days
    };

    // Local dates of a history period, both included
    struct HistoryRange
    {
        QDate first;
        QDate last;

        bool operator==(const HistoryRange &) const = default;
    };

    struct HistoryPeriods
    {
        // Weeks start on Monday, as the totals; Last30Days ends today
        static HistoryRange range(HistoryPeriod period, const QDate &today);

        // Stored and QML form: "week", "month", "last30"; an unknown key is ThisWeek
        static QString       keyOf(HistoryPeriod period);
        static HistoryPeriod fromKey(const QString &key);
    };
} // namespace kemai
