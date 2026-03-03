// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// Qt headers
#include <QObject>
#include <QtQmlIntegration/qqmlintegration.h>

namespace kemai
{
    class ApplicationContext : public QObject
    {
        Q_OBJECT
        QML_ELEMENT

    public:
        explicit ApplicationContext(QObject *parent = nullptr);
        ~ApplicationContext() override;

        Q_INVOKABLE void initialize();
        Q_INVOKABLE void login(const QString &host, const QString &token);
    };
} // namespace kemai
