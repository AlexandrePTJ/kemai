// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// STL headers
#include <memory>
#include <optional>

// Qt headers
#include <QObject>
#include <QTimer>

// Project headers
#include <client/iKimaiClient.h>
#include <client/kimaiApiError.h>
#include <client/kimaiCache.h>
#include <client/kimaiSystem.h>
#include <client/kimaiTimesheets.h>
#include <model/activityListModel.h>
#include <model/timesheetModel.h>

namespace kemai
{
    /*!
     * Business logic for a logged-in session: models, cache, active timesheet
     * tracking, and the error policy (401 -> sessionExpired, everything else ->
     * notificationRequested). The QML-facing SessionContext derives from it.
     */
    class SessionController : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(QString username READ username CONSTANT)
        Q_PROPERTY(kemai::TimesheetModel *recentTimeSheets READ recentTimeSheets CONSTANT)
        Q_PROPERTY(bool hasActiveTimesheet READ hasActiveTimesheet NOTIFY activeTimesheetChanged)
        Q_PROPERTY(QString activeTimesheetLabel READ activeTimesheetLabel NOTIFY activeTimesheetChanged)
        Q_PROPERTY(QString activeTimesheetDuration READ activeTimesheetDuration NOTIFY activeTimesheetChanged)
        Q_PROPERTY(kemai::ActivityListModel *activityModel READ activityModel CONSTANT)

    public:
        enum class NotificationLevel
        {
            Info,
            Warning,
            Error
        };
        Q_ENUM(NotificationLevel)

        enum class NotificationKind
        {
            NoNetwork,
            Forbidden,
            NotFound,
            ServerError,
            InvalidResponse,
            UnknownError,
            CacheLoadFailed
        };
        Q_ENUM(NotificationKind)

        explicit SessionController(std::shared_ptr<IKimaiClient> client, const KimaiUser &user, QObject *parent = nullptr);
        ~SessionController() override;

        QString            username() const;
        TimesheetModel    *recentTimeSheets() const;
        bool               hasActiveTimesheet() const;
        QString            activeTimesheetLabel() const;
        QString            activeTimesheetDuration() const;
        ActivityListModel *activityModel() const;

        Q_INVOKABLE void startTimeSheet(int activityId, int projectId);
        Q_INVOKABLE void stopActiveTimeSheet();
        Q_INVOKABLE void updateTimeSheetTimes(int timesheetId, const QDateTime &beginAt, const QDateTime &endAt);

    signals:
        void activeTimesheetChanged();
        // Auth is no longer valid (401): the UI must drop the session and return to login.
        void sessionExpired();
        // Single UI notification channel; the QML banner composes the human message from kind.
        void notificationRequested(kemai::SessionController::NotificationLevel level, kemai::SessionController::NotificationKind kind, const QString &detail);

    private:
        void refreshRecentTimeSheets();
        void refreshActiveTimeSheets();
        void onCacheLoaded();
        void onCacheLoadFailed();

        // Error policy entry point: log + 401 -> sessionExpired, otherwise notify the UI.
        void handleApiError(const KimaiApiError &error, const char *context);

        std::shared_ptr<IKimaiClient>      m_client;
        KimaiUser                          m_user;
        std::unique_ptr<TimesheetModel>    m_recentTimeSheets;
        std::unique_ptr<ActivityListModel> m_activityModel;
        std::unique_ptr<KimaiCache>        m_cache;
        std::optional<KimaiTimeSheet>      m_activeTimeSheet;
        QTimer                             m_recentRefreshTimer;
        QTimer                             m_activeDurationTimer;
    };
} // namespace kemai
