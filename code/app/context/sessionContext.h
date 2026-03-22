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
        QML_UNCREATABLE("SessionContext is created by the application")

        Q_PROPERTY(QString username READ username CONSTANT)

    public:
        explicit SessionContext(std::unique_ptr<KimaiClient> client, const KimaiUser &user, QObject *parent = nullptr);
        ~SessionContext() override;

        QString username() const;

    private:
        std::unique_ptr<KimaiClient> m_client;
        KimaiUser                    m_user;
    };
} // namespace kemai
