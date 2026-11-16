// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "formatHelpers.h"

// Qt headers
#include <QLocale>

namespace
{
    // explicitToday: name today ("Today") instead of omitting it
    QString formatDay(const QDate &date, const QDate &today, bool explicitToday)
    {
        if (date == today)
        {
            return explicitToday ? QStringLiteral("Today") : QString{};
        }
        if (date == today.addDays(-1))
        {
            return QStringLiteral("Yesterday");
        }
        return QLocale().toString(date, QStringLiteral("ddd, MMM d"));
    }

    QString withDay(const QString &day, const QString &time)
    {
        return day.isEmpty() ? time : QStringLiteral("%1 %2").arg(day, time);
    }
} // namespace

namespace kemai
{
    QString FormatHelpers::formatDuration(qint64 totalSeconds)
    {
        if (totalSeconds < 0)
        {
            totalSeconds = 0;
        }

        const auto hours   = totalSeconds / 3600;
        const auto minutes = (totalSeconds % 3600) / 60;
        const auto seconds = totalSeconds % 60;

        return QStringLiteral("%1:%2:%3")
            .arg(hours, 2, 10, QLatin1Char('0'))
            .arg(minutes, 2, 10, QLatin1Char('0'))
            .arg(seconds, 2, 10, QLatin1Char('0'));
    }

    QString FormatHelpers::formatDurationShort(qint64 totalSeconds)
    {
        if (totalSeconds < 0)
        {
            totalSeconds = 0;
        }

        const auto hours   = totalSeconds / 3600;
        const auto minutes = (totalSeconds % 3600) / 60;

        return QStringLiteral("%1h %2m")
            .arg(hours)
            .arg(minutes, 2, 10, QLatin1Char('0'));
    }

    QString FormatHelpers::formatTimeRange(const QDateTime &beginAt, const QDateTime &endAt, const QDate &today)
    {
        const auto timeFormat = QStringLiteral("HH:mm");
        const auto begin      = beginAt.toLocalTime();
        const auto beginText  = withDay(formatDay(begin.date(), today, false), begin.toString(timeFormat));

        if (!endAt.isValid())
        {
            return QStringLiteral("%1 \u2013 now").arg(beginText);
        }

        const auto end     = endAt.toLocalTime();
        const auto endTime = end.toString(timeFormat);
        const auto endText = end.date() == begin.date() ? endTime : withDay(formatDay(end.date(), today, true), endTime);
        return QStringLiteral("%1 \u2013 %2").arg(beginText, endText);
    }
} // namespace kemai
