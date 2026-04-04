// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "formatHelpers.h"

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
} // namespace kemai
