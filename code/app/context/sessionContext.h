// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// STL headers
#include <memory>

// Qt headers
#include <QObject>
#include <QTimer>
#include <QtQmlIntegration/qqmlintegration.h>

// Project headers
#include "client/kimaiClient.h"
#include "client/kimaiSystem.h"
#include "model/timesheetModel.h"

namespace kemai
{
    class SessionContext : public QObject
    {
        Q_OBJECT
        QML_ELEMENT
        QML_UNCREATABLE("SessionContext is created by the application")

        Q_PROPERTY(QString username READ username CONSTANT)
        Q_PROPERTY(kemai::TimesheetModel *recentTimeSheets READ recentTimeSheets CONSTANT)

    public:
        explicit SessionContext(std::shared_ptr<KimaiClient> client, const KimaiUser &user, QObject *parent = nullptr);
        ~SessionContext() override;

        QString         username() const;
        TimesheetModel *recentTimeSheets() const;

    private:
        void refreshRecentTimeSheets();

        std::shared_ptr<KimaiClient>    m_client;
        KimaiUser                       m_user;
        std::unique_ptr<TimesheetModel> m_recentTimeSheets;
        QTimer                          m_activeEntryTimer;
    };
} // namespace kemai
