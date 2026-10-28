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
#include <QUrl>

// Project headers
#include <client/iKimaiClient.h>
#include <client/kimaiApiError.h>
#include <client/kimaiCache.h>
#include <client/kimaiSystem.h>
#include <client/kimaiTimesheets.h>
#include <misc/customFields.h>
#include <model/activityListModel.h>
#include <model/taskModel.h>
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
        // The instance's web interface: the host as typed, with https:// when no scheme was given
        Q_PROPERTY(QUrl webUrl READ webUrl NOTIFY accountChanged)
        Q_PROPERTY(kemai::TimesheetModel *recentTimeSheets READ recentTimeSheets CONSTANT)
        Q_PROPERTY(bool hasActiveTimesheet READ hasActiveTimesheet NOTIFY activeTimesheetChanged)
        Q_PROPERTY(QString activeTimesheetLabel READ activeTimesheetLabel NOTIFY activeTimesheetChanged)
        Q_PROPERTY(QString activeTimesheetDuration READ activeTimesheetDuration NOTIFY activeTimesheetChanged)
        Q_PROPERTY(QString activeProjectName READ activeProjectName NOTIFY activeTimesheetChanged)
        Q_PROPERTY(QString activeActivityName READ activeActivityName NOTIFY activeTimesheetChanged)
        Q_PROPERTY(QString activeActivityColor READ activeActivityColor NOTIFY activeTimesheetChanged)
        // Local start time of the running timer, invalid without one
        Q_PROPERTY(QDateTime activeTimesheetBegin READ activeTimesheetBegin NOTIFY activeTimesheetChanged)
        Q_PROPERTY(kemai::ActivityListModel *activityModel READ activityModel CONSTANT)
        Q_PROPERTY(bool refreshing READ isRefreshing NOTIFY refreshingChanged)
        Q_PROPERTY(bool punchMode READ isPunchMode NOTIFY punchModeChanged)
        // Task Management plugin installed on the instance: the tasks view exists only then
        Q_PROPERTY(bool hasTaskManagement READ hasTaskManagement NOTIFY pluginsChanged)
        Q_PROPERTY(kemai::TaskModel *tasks READ tasks CONSTANT)
        Q_PROPERTY(kemai::SessionController::TasksLoad tasksLoad READ tasksLoad NOTIFY tasksLoadChanged)
        // Last load failure (tasksLoad Failed)
        Q_PROPERTY(kemai::SessionController::NotificationKind tasksError READ tasksError NOTIFY tasksLoadChanged)
        // IANA timezone of the Kimai user (prefill of new customers)
        Q_PROPERTY(QString userTimezone READ userTimezone CONSTANT)

    public:
        enum class NotificationLevel
        {
            Info,
            Warning,
            Error,
            Success
        };
        Q_ENUM(NotificationLevel)

        enum class NotificationKind
        {
            NoNetwork,
            Forbidden,
            NotFound,
            ServerError,
            InvalidResponse,
            InvalidValue,         // detail: Kimai's message
            AuthFailed,           // login only (in a session, 401 ends it): token refused
            HttpsRedirect,        // detail: the https address the http:// host redirects to
            UntrustedCertificate, // login only: the server certificate can be trusted (KemaiContext::refusedCertificate)
            CertificateRejected,  // login only: the server certificate cannot be trusted (expired, other name…)
            UnknownError,
            CacheLoadFailed,    // initial load: no usable cache
            CacheRefreshFailed, // refresh: previous cache kept
            TimerStoppedIdle,   // detail: end time, reported when the user is back
            TimerStoppedLocked,
            ResourceCreated // detail: "activity|name|project" (project empty for a global one) or "project|name|"
        };
        Q_ENUM(NotificationKind)

        // Why the timer was stopped while the user was away
        enum class AwayReason
        {
            Idle,
            Locked
        };
        Q_ENUM(AwayReason)

        enum class TasksLoad
        {
            NotLoaded,
            Loading, // first load only: later reloads keep the list (Ready) meanwhile
            Ready,
            Failed
        };
        Q_ENUM(TasksLoad)

        enum class ResourceKind
        {
            Customer,
            Project,
            Activity
        };
        Q_ENUM(ResourceKind)

        explicit SessionController(std::shared_ptr<IKimaiClient> client, const KimaiUser &user, QObject *parent = nullptr);

        // The UI kind of an API error (also used for login failures, before any session)
        static NotificationKind notificationKindOf(ApiErrorKind kind);
        ~SessionController() override;

        QString username() const;
        // Saved profile name the session was opened from, else the Kimai user name
        QString            accountName() const;
        QString            host() const;
        QUrl               webUrl() const;
        TimesheetModel    *recentTimeSheets() const;
        bool               hasActiveTimesheet() const;
        QString            activeTimesheetLabel() const;
        QString            activeTimesheetDuration() const;
        QString            activeProjectName() const;
        QString            activeActivityName() const;
        QString            activeActivityColor() const;
        QDateTime          activeTimesheetBegin() const;
        ActivityListModel *activityModel() const;
        bool               isRefreshing() const;
        // Kimai manages begin/end itself: they are never sent on update
        bool             isPunchMode() const;
        QString          userTimezone() const;
        bool             hasTaskManagement() const;
        TaskModel       *tasks() const;
        TasksLoad        tasksLoad() const;
        NotificationKind tasksError() const;
        // Project of the most recent entry (prefill of new activities), 0 without entries
        Q_INVOKABLE int lastUsedProjectId() const;
        // The entry's page in Kimai's web interface (in the Kimai user's language)
        Q_INVOKABLE QUrl timeSheetWebUrl(int timesheetId) const;

        // Where the session comes from, for display: `profileName` may be empty (credentials typed by hand)
        void setAccount(const QString &profileName, const QString &host);

        // Cache age beyond which refreshIfStale() reloads it.
        void setStaleAfter(std::chrono::seconds staleAfter);

        // Starts a timer at once. Required custom fields get their default value, or the value they have in
        // `sourceTimesheetId` (a history entry restarted); a failed write emits customFieldsSaveFailed.
        Q_INVOKABLE void startTimeSheet(int activityId, int projectId, int sourceTimesheetId = 0);
        // Starts a timer from the dialog's new entry mode, then writes `customValues` (name -> value). Answered by
        // timeSheetSaved(new id), or timeSheetSaveFailed(0, ...) when nothing started, or customFieldsSaveFailed.
        Q_INVOKABLE void startTimeSheetWith(int projectId, int activityId, const QString &description, const QStringList &tags, const QStringList &newTags, const QVariantMap &customValues);
        // Whether starting on that project and activity would leave a required custom field Kemai can fill empty
        // or invalid, after defaults and the values of `sourceTimesheetId` (0: none): the UI then asks for them first
        Q_INVOKABLE bool needsCustomFields(int projectId, int activityId, int sourceTimesheetId = 0) const;
        // Codes a country, currency or language field may hold (given by the app, which has the lists)
        void                            setKnownCodes(const CustomFields::KnownCodes &codes);
        const CustomFields::KnownCodes &knownCodes() const;
        // Required custom fields of time entries and the palette of color fields, from the cache
        const KimaiMetaFieldRules &customFieldRules() const;
        const KimaiColorChoices   &colorChoices() const;
        // Customer of a cached project (id 0 when the project is unknown)
        KimaiCustomer    customerOfProject(int projectId) const;
        Q_INVOKABLE void stopActiveTimeSheet();
        // Stops the running timesheet with `end` as its end (the user left at that time): applied through an update,
        // except in punch mode or when `end` is not after the begin, where it is a plain stop. Answered by
        // timeSheetStoppedAway on success; a failure goes through the error policy.
        Q_INVOKABLE void stopActiveTimeSheetAt(const QDateTime &end, kemai::SessionController::AwayReason reason);
        // Applies every field as given, except begin/end: an invalid QDateTime leaves it unchanged,
        // and a running entry keeps no end. Returns false (nothing sent) when end <= begin or project/activity
        // are invalid; otherwise answered by timeSheetSaved or timeSheetSaveFailed.
        // `newTags` (among `tags`) do not exist on the server yet: they are created before the update.
        // `customValues` (name -> value) are written after the update, only those that differ from the server's.
        Q_INVOKABLE bool updateTimeSheet(int timesheetId, int projectId, int activityId, const QString &description, const QStringList &tags, const QDateTime &beginAt, const QDateTime &endAt, const QStringList &newTags = {}, const QVariantMap &customValues = {});
        // Writes only the custom values of an existing entry (Retry after a partial failure), those that differ from
        // the server's as far as the history knows them. Answered by timeSheetSaved or customFieldsSaveFailed.
        Q_INVOKABLE void saveCustomValues(int timesheetId, int projectId, int activityId, const QVariantMap &customValues);
        // Deletes the timesheet on the server, then refreshes active + recent (stops tracking if it was active).
        // Returns false (nothing sent) for an invalid id; otherwise answered by timeSheetDeleted or timeSheetDeleteFailed.
        Q_INVOKABLE bool deleteTimeSheet(int timesheetId);
        // Existing server tags containing `text`; answered by tagSuggestionsReady
        Q_INVOKABLE void requestTagSuggestions(const QString &text);

        // Create a Kimai customer / project / activity (projectId 0: global activity). Answered by resourceCreated
        // with the new id, or resourceCreateFailed for the form to show (401 still emits sessionExpired). Nothing is
        // reloaded: after the last step of a creation, reloadResources() makes the new items searchable.
        Q_INVOKABLE void createCustomer(const QString &name, const QString &country, const QString &currency, const QString &timezone);
        Q_INVOKABLE void createProject(const QString &name, int customerId);
        Q_INVOKABLE void createActivity(const QString &name, int projectId);
        // Reloads customers, projects and activities (unless a load is in flight); `refreshing` follows it
        Q_INVOKABLE void reloadResources();

        // Task Management: (re)loads the user's open tasks; also done when the running timer changes once loaded
        Q_INVOKABLE void loadTasks();
        // Starts a timer on the task (Kimai stops a running one when only one may run), titled after the task.
        // Answered by taskStarted, or taskActionFailed(id, "start", ...) for the row to show (401 → sessionExpired)
        Q_INVOKABLE void startTask(int taskId);
        // startTask from the dialog's new entry mode, then writes `customValues`. Answered by taskStarted and
        // timeSheetSaved(new id), or timeSheetSaveFailed(0, ...) when the task did not start, or customFieldsSaveFailed.
        Q_INVOKABLE void startTaskWith(int taskId, const QVariantMap &customValues);
        // Marks the task done (Kimai also stops its running timer). Answered by taskClosed, or taskActionFailed(id, "close", ...)
        Q_INVOKABLE void closeTask(int taskId);

        // Choices for the creation forms, from the cache, sorted by name:
        // customers {id, name, projectCount}, projects {id, name, customerId, customerName, color},
        // global activities {id, name, color}
        Q_INVOKABLE QVariantList customerChoices() const;
        Q_INVOKABLE QVariantList projectChoices() const;
        Q_INVOKABLE QVariantList globalActivityChoices() const;
        // Existing item with that name (trimmed, case-insensitive), as Kimai accepts duplicates: {id, name, ...} or empty.
        // findActivity looks in the project (projectId > 0) or among global activities (0).
        Q_INVOKABLE QVariantMap findCustomer(const QString &name) const;
        Q_INVOKABLE QVariantMap findProject(const QString &name, int customerId) const;
        Q_INVOKABLE QVariantMap findActivity(const QString &name, int projectId) const;
        // A project — activity pair as the pickers show it: {projectId, activityId, projectName, activityName,
        // activityColor, label, isGlobal}, or empty when the cache does not hold it
        Q_INVOKABLE QVariantMap pair(int projectId, int activityId) const;

        // Reloads the cache (unless a load is in flight) and the timesheets.
        Q_INVOKABLE void refresh();
        // refresh() only when the cache is missing or older than the stale threshold.
        Q_INVOKABLE void refreshIfStale();

        // Checks the running timer on the server (also every syncInterval): a timer started, stopped or changed
        // elsewhere updates the active state and reloads the history; an unchanged one causes no visible refresh.
        Q_INVOKABLE void syncActiveTimeSheet();
        void             setSyncInterval(std::chrono::milliseconds interval);

        static constexpr std::chrono::seconds DefaultSyncInterval{30};

    signals:
        void activeTimesheetChanged();
        void accountChanged();
        void refreshingChanged();
        void punchModeChanged();
        // Update/delete outcomes, for the edit dialog that shows its own failures (never 401: sessionExpired instead)
        void timeSheetSaved(int timesheetId);
        void timeSheetSaveFailed(int timesheetId, kemai::SessionController::NotificationKind kind, const QString &detail);
        // Writing custom values of the (existing) entry stopped at the first failure; `pendingNames` were not written
        void customFieldsSaveFailed(int timesheetId, kemai::SessionController::NotificationKind kind, const QString &detail, const QStringList &pendingNames);
        void timeSheetDeleted(int timesheetId);
        void timeSheetDeleteFailed(int timesheetId, kemai::SessionController::NotificationKind kind, const QString &detail);
        void pluginsChanged();
        void tasksLoadChanged();
        void taskStarted(int taskId);
        void taskClosed(int taskId);
        void taskActionFailed(int taskId, const QString &action, kemai::SessionController::NotificationKind error, const QString &detail);
        void resourceCreated(kemai::SessionController::ResourceKind kind, int id);
        // `field`: the Kimai field a refused value belongs to, when known
        void resourceCreateFailed(kemai::SessionController::ResourceKind kind, kemai::SessionController::NotificationKind error, const QString &detail, const QString &field);
        // `query` is the text the suggestions answer, so the UI can drop stale replies
        void tagSuggestionsReady(const QString &query, const QStringList &tags);
        // The running timesheet was stopped by stopActiveTimeSheetAt; `end` is the end it got
        void timeSheetStoppedAway(kemai::SessionController::AwayReason reason, const QDateTime &end);
        // Auth is no longer valid (401): the UI must drop the session and return to login.
        void sessionExpired();
        // Single UI notification channel; the QML banner composes the human message from kind.
        void notificationRequested(kemai::SessionController::NotificationLevel level, kemai::SessionController::NotificationKind kind, const QString &detail);

    private:
        void refreshRecentTimeSheets();
        void refreshActiveTimeSheets();
        // Takes the server's active timesheets as the current state; returns whether it differs from the previous one
        bool applyActiveTimeSheets(const KimaiTimeSheets &sheets);
        // Called when a local start/stop/update/delete is sent: sync replies sent before it are outdated
        void invalidateSync();
        // Creates `names` one after the other, then runs `next`; a failure reports timeSheetSaveFailed instead
        void createTagsThen(QStringList names, int timesheetId, std::function<void()> next);
        using CustomValues = QMap<QString, QString>;
        using CustomWrites = QList<std::pair<QString, QString>>;
        QList<CustomFields::Field> resolveCustomFields(int projectId, int activityId, const CustomValues &values) const;
        // Values of `wanted` for the fields that apply and Kemai can fill, which differ from `current`
        CustomWrites customWrites(int projectId, int activityId, const CustomValues &wanted, const CustomValues &current) const;
        // Starts a task, titles its timesheet after it, then runs `next` with the started timesheet
        void startTaskThen(int taskId, std::function<void(const KimaiTimeSheet &)> next, std::function<void(const KimaiApiError &)> failed);
        // Writes `pending` one after the other, then runs `done`; the first failure emits customFieldsSaveFailed
        void writeCustomValues(int timesheetId, CustomWrites pending, std::function<void()> done);
        void handleCreateError(const KimaiApiError &error, ResourceKind kind);
        void handleTaskError(const KimaiApiError &error, int taskId, const QString &action);
        void requestPlugins();
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
        QTimer                             m_syncTimer;
        bool                               m_syncInFlight      = false;
        bool                               m_syncFailing       = false;
        int                                m_syncGeneration    = 0;
        std::chrono::seconds               m_staleAfter        = std::chrono::minutes(10);
        int                                m_lastUsedProjectId = 0;
        std::unique_ptr<TaskModel>         m_tasks;
        bool                               m_hasTaskManagement = false;
        TasksLoad                          m_tasksLoad         = TasksLoad::NotLoaded;
        NotificationKind                   m_tasksError        = NotificationKind::UnknownError;
        int                                m_tasksGeneration   = 0;
        CustomFields::KnownCodes           m_knownCodes;
    };
} // namespace kemai
