// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "qmlFormatHelpers.h"

// Project headers
#include <misc/formatHelpers.h>

namespace kemai
{
    QmlFormatHelpers::QmlFormatHelpers(QObject *parent):
    QObject(parent)
    {
    }

    QString QmlFormatHelpers::formatDuration(qint64 totalSeconds)
    {
        return FormatHelpers::formatDuration(totalSeconds);
    }
} // namespace kemai
