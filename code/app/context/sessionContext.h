// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// STL headers
#include <memory>
#include <optional>

// Qt headers
#include <QObject>
#include <QTimer>
#include <QtQmlIntegration/qqmlintegration.h>

// Project headers
#include <client/kimaiCache.h>
#include <client/kimaiClient.h>
#include <client/kimaiSystem.h>
#include <client/kimaiTimesheets.h>
#include <model/timesheetModel.h>

namespace kemai
{
    class SessionContext : public QObject
    {
        Q_OBJECT
        QML_ELEMENT
        QML_UNCREATABLE("SessionContext is created by the application")

        Q_PROPERTY(QString username READ username CONSTANT)
        Q_PROPERTY(kemai::TimesheetModel *recentTimeSheets READ recentTimeSheets CONSTANT)
        Q_PROPERTY(bool hasActiveTimesheet READ hasActiveTimesheet NOTIFY activeTimesheetChanged)
        Q_PROPERTY(QString activeTimesheetLabel READ activeTimesheetLabel NOTIFY activeTimesheetChanged)
        Q_PROPERTY(QString activeTimesheetDuration READ activeTimesheetDuration NOTIFY activeTimesheetChanged)

    public:
        explicit SessionContext(std::shared_ptr<KimaiClient> client, const KimaiUser &user, QObject *parent = nullptr);
        ~SessionContext() override;

        QString         username() const;
        TimesheetModel *recentTimeSheets() const;
        bool            hasActiveTimesheet() const;
        QString         activeTimesheetLabel() const;
        QString         activeTimesheetDuration() const;

    signals:
        void activeTimesheetChanged();

    private:
        void refreshRecentTimeSheets();
        void refreshActiveTimeSheets();

        std::shared_ptr<KimaiClient>    m_client;
        KimaiUser                       m_user;
        std::unique_ptr<TimesheetModel> m_recentTimeSheets;
        std::unique_ptr<KimaiCache>     m_cache;
        std::optional<KimaiTimeSheet>   m_activeTimeSheet;
        QTimer                          m_recentRefreshTimer;
        QTimer                          m_activeDurationTimer;
    };
} // namespace kemai
