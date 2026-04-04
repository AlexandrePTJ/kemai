// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// Qt headers
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
    };
} // namespace kemai
