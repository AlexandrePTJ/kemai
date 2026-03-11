// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// STL headers
#include <memory>

// Qt headers
#include <QObject>
#include <QtQmlIntegration/qqmlintegration.h>

// Project headers
#include "client/kimaiClient.h"
#include "client/kimaiSystem.h"

namespace kemai
{
    class SessionContext : public QObject
    {
        Q_OBJECT
        QML_ELEMENT

        Q_PROPERTY(QString host READ host CONSTANT)
        Q_PROPERTY(QString username READ username CONSTANT)

    public:
        explicit SessionContext(std::unique_ptr<KimaiClient> client, const QString &host, const KimaiUser &user, QObject *parent = nullptr);
        ~SessionContext() override;

        QString host() const;
        QString username() const;

    private:
        std::unique_ptr<KimaiClient> m_client;
        QString                      m_host;
        KimaiUser                    m_user;
    };
} // namespace kemai
