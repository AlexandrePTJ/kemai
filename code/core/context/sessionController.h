// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// STL headers
#include <chrono>
#include <functional>
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
        Q_PROPERTY(QString accountName READ accountName NOTIFY accountChanged)
        Q_PROPERTY(QString host READ host NOTIFY accountChanged)
        Q_PROPERTY(kemai::TimesheetModel *recentTimeSheets READ recentTimeSheets CONSTANT)
        Q_PROPERTY(bool hasActiveTimesheet READ hasActiveTimesheet NOTIFY activeTimesheetChanged)
        Q_PROPERTY(QString activeTimesheetLabel READ activeTimesheetLabel NOTIFY activeTimesheetChanged)
        Q_PROPERTY(QString activeTimesheetDuration READ activeTimesheetDuration NOTIFY activeTimesheetChanged)
        Q_PROPERTY(QString activeProjectName READ activeProjectName NOTIFY activeTimesheetChanged)
        Q_PROPERTY(QString activeActivityName READ activeActivityName NOTIFY activeTimesheetChanged)
        Q_PROPERTY(QString activeActivityColor READ activeActivityColor NOTIFY activeTimesheetChanged)
        Q_PROPERTY(kemai::ActivityListModel *activityModel READ activityModel CONSTANT)
        Q_PROPERTY(bool refreshing READ isRefreshing NOTIFY refreshingChanged)
        Q_PROPERTY(bool punchMode READ isPunchMode NOTIFY punchModeChanged)

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
            CacheLoadFailed,   // initial load: no usable cache
            CacheRefreshFailed // refresh: previous cache kept
        };
        Q_ENUM(NotificationKind)

        explicit SessionController(std::shared_ptr<IKimaiClient> client, const KimaiUser &user, QObject *parent = nullptr);
        ~SessionController() override;

        QString            username() const;
        // Saved profile name the session was opened from, else the Kimai user name
        QString            accountName() const;
        QString            host() const;
        TimesheetModel    *recentTimeSheets() const;
        bool               hasActiveTimesheet() const;
        QString            activeTimesheetLabel() const;
        QString            activeTimesheetDuration() const;
        QString            activeProjectName() const;
        QString            activeActivityName() const;
        QString            activeActivityColor() const;
        ActivityListModel *activityModel() const;
        bool               isRefreshing() const;
        // Kimai manages begin/end itself: they are never sent on update
        bool               isPunchMode() const;

        // Where the session comes from, for display: `profileName` may be empty (credentials typed by hand)
        void setAccount(const QString &profileName, const QString &host);

        // Cache age beyond which refreshIfStale() reloads it.
        void setStaleAfter(std::chrono::seconds staleAfter);

        Q_INVOKABLE void startTimeSheet(int activityId, int projectId);
        Q_INVOKABLE void stopActiveTimeSheet();
        // Applies every field as given, except begin/end: an invalid QDateTime leaves it unchanged,
        // and a running entry keeps no end. Returns false (nothing sent) when end <= begin or project/activity
        // are invalid; otherwise answered by timeSheetSaved or timeSheetSaveFailed.
        // `newTags` (among `tags`) do not exist on the server yet: they are created before the update.
        Q_INVOKABLE bool updateTimeSheet(int timesheetId, int projectId, int activityId, const QString &description, const QStringList &tags,
                                         const QDateTime &beginAt, const QDateTime &endAt, const QStringList &newTags = {});
        // Deletes the timesheet on the server, then refreshes active + recent (stops tracking if it was active).
        // Returns false (nothing sent) for an invalid id; otherwise answered by timeSheetDeleted or timeSheetDeleteFailed.
        Q_INVOKABLE bool deleteTimeSheet(int timesheetId);
        // Existing server tags containing `text`; answered by tagSuggestionsReady
        Q_INVOKABLE void requestTagSuggestions(const QString &text);

        // Reloads the cache (unless a load is in flight) and the timesheets.
        Q_INVOKABLE void refresh();
        // refresh() only when the cache is missing or older than the stale threshold.
        Q_INVOKABLE void refreshIfStale();

    signals:
        void activeTimesheetChanged();
        void accountChanged();
        void refreshingChanged();
        void punchModeChanged();
        // Update/delete outcomes, for the edit dialog that shows its own failures (never 401: sessionExpired instead)
        void timeSheetSaved(int timesheetId);
        void timeSheetSaveFailed(int timesheetId, kemai::SessionController::NotificationKind kind, const QString &detail);
        void timeSheetDeleted(int timesheetId);
        void timeSheetDeleteFailed(int timesheetId, kemai::SessionController::NotificationKind kind, const QString &detail);
        // `query` is the text the suggestions answer, so the UI can drop stale replies
        void tagSuggestionsReady(const QString &query, const QStringList &tags);
        // Auth is no longer valid (401): the UI must drop the session and return to login.
        void sessionExpired();
        // Single UI notification channel; the QML banner composes the human message from kind.
        void notificationRequested(kemai::SessionController::NotificationLevel level, kemai::SessionController::NotificationKind kind, const QString &detail);

    private:
        void refreshRecentTimeSheets();
        void refreshActiveTimeSheets();
        // Creates `names` one after the other, then runs `next`; a failure reports timeSheetSaveFailed instead
        void createTagsThen(QStringList names, int timesheetId, std::function<void()> next);
        void onCacheLoaded();
        void onCacheLoadFailed();

        // Error policy entry point: log + 401 -> sessionExpired, otherwise notify the UI.
        void handleApiError(const KimaiApiError &error, const char *context);
        // Same policy, but the failure goes back to the caller through `failed` instead of the banner.
        template<typename FailedSignal>
        void handleOperationError(const KimaiApiError &error, const char *context, int timesheetId, FailedSignal failed);

        std::shared_ptr<IKimaiClient>      m_client;
        KimaiUser                          m_user;
        QString                            m_profileName;
        QString                            m_host;
        std::unique_ptr<TimesheetModel>    m_recentTimeSheets;
        std::unique_ptr<ActivityListModel> m_activityModel;
        std::unique_ptr<KimaiCache>        m_cache;
        std::optional<KimaiTimeSheet>      m_activeTimeSheet;
        QTimer                             m_recentRefreshTimer;
        QTimer                             m_activeDurationTimer;
        std::chrono::seconds               m_staleAfter = std::chrono::minutes(10);
    };
} // namespace kemai
