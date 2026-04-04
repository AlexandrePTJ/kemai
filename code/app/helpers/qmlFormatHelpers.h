// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// Qt headers
#include <QObject>
#include <QtQmlIntegration/qqmlintegration.h>

namespace kemai
{
    class QmlFormatHelpers : public QObject
    {
        Q_OBJECT
        QML_ELEMENT
        QML_SINGLETON
        QML_NAMED_ELEMENT(FormatHelpers)

    public:
        explicit QmlFormatHelpers(QObject *parent = nullptr);

        Q_INVOKABLE static QString formatDuration(qint64 totalSeconds);
    };
} // namespace kemai
