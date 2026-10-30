// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "historyPeriods.h"

namespace kemai
{
    HistoryRange HistoryPeriods::range(HistoryPeriod period, const QDate &today)
    {
        switch (period)
        {
        case HistoryPeriod::ThisMonth:
        {
            const QDate first(today.year(), today.month(), 1);
            return {first, first.addMonths(1).addDays(-1)};
        }

        case HistoryPeriod::Last30Days:
            return {today.addDays(-29), today};

        case HistoryPeriod::ThisWeek:
        default:
        {
            const auto monday = today.addDays(-(today.dayOfWeek() - 1));
            return {monday, monday.addDays(6)};
        }
        }
    }

    QString HistoryPeriods::keyOf(HistoryPeriod period)
    {
        switch (period)
        {
        case HistoryPeriod::ThisMonth:
            return QStringLiteral("month");
        case HistoryPeriod::Last30Days:
            return QStringLiteral("last30");
        case HistoryPeriod::ThisWeek:
        default:
            return QStringLiteral("week");
        }
    }

    HistoryPeriod HistoryPeriods::fromKey(const QString &key)
    {
        if (key == QLatin1String("month"))
        {
            return HistoryPeriod::ThisMonth;
        }
        if (key == QLatin1String("last30"))
        {
            return HistoryPeriod::Last30Days;
        }
        return HistoryPeriod::ThisWeek;
    }
} // namespace kemai
