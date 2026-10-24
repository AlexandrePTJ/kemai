// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "sessionController.h"

// Qt headers
#include <QSet>

// STL headers
#include <algorithm>

// 3rd party headers
#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>

// Project headers
#include <misc/customFmt.h>
#include <misc/formatHelpers.h>

namespace
{
    kemai::SessionController::NotificationKind notificationKindFromError(kemai::ApiErrorKind kind)
    {
        using kemai::ApiErrorKind;
        using Kind = kemai::SessionController::NotificationKind;

        switch (kind)
        {
        case ApiErrorKind::NoNetwork:
            return Kind::NoNetwork;
        case ApiErrorKind::Forbidden:
            return Kind::Forbidden;
        case ApiErrorKind::NotFound:
            return Kind::NotFound;
        case ApiErrorKind::ServerError:
            return Kind::ServerError;
        case ApiErrorKind::InvalidResponse:
            return Kind::InvalidResponse;
        case ApiErrorKind::InvalidValue:
            return Kind::InvalidValue;
        case ApiErrorKind::AuthFailed:
            return Kind::AuthFailed;
        case ApiErrorKind::HttpsRedirect:
            return Kind::HttpsRedirect;
        default:
            return Kind::UnknownError;
        }
    }
} // namespace

namespace kemai
{
    SessionController::SessionController(std::shared_ptr<IKimaiClient> client, const KimaiUser &user, QObject *parent):
    QObject(parent),
    m_client(std::move(client)),
    m_user(user),
    m_recentTimeSheets(std::make_unique<TimesheetModel>(this)),
    m_activityModel(std::make_unique<ActivityListModel>(this)),
    m_cache(std::make_unique<KimaiCache>(this)),
    m_tasks(std::make_unique<TaskModel>(this))
    {
        m_recentRefreshTimer.setInterval(std::chrono::minutes(1));
        connect(&m_recentRefreshTimer, &QTimer::timeout, m_recentTimeSheets.get(), &TimesheetModel::refreshActiveDurations);

        m_activeDurationTimer.setInterval(std::chrono::seconds(1));
        connect(&m_activeDurationTimer, &QTimer::timeout, this, &SessionController::activeTimesheetChanged);

        m_syncTimer.setInterval(DefaultSyncInterval);
        connect(&m_syncTimer, &QTimer::timeout, this, &SessionController::syncActiveTimeSheet);
        m_syncTimer.start();

        connect(m_cache.get(), &KimaiCache::loaded, this, &SessionController::onCacheLoaded);
        connect(m_cache.get(), &KimaiCache::loadFailed, this, &SessionController::onCacheLoadFailed);

        refresh();
        requestPlugins();
    }

    SessionController::NotificationKind SessionController::notificationKindOf(ApiErrorKind kind)
    {
        return notificationKindFromError(kind);
    }

    SessionController::~SessionController()
    {
        spdlog::debug("Session of {} closed", m_user.username);
    }

    QString SessionController::username() const
    {
        return m_user.username;
    }

    QString SessionController::accountName() const
    {
        return m_profileName.isEmpty() ? m_user.username : m_profileName;
    }

    QString SessionController::host() const
    {
        return m_host;
    }

    void SessionController::setAccount(const QString &profileName, const QString &host)
    {
        m_profileName = profileName;
        m_host        = host;
        emit accountChanged();
    }

    TimesheetModel *SessionController::recentTimeSheets() const
    {
        return m_recentTimeSheets.get();
    }

    bool SessionController::hasActiveTimesheet() const
    {
        return m_activeTimeSheet.has_value();
    }

    QString SessionController::activeTimesheetLabel() const
    {
        if (!m_activeTimeSheet)
        {
            return {};
        }
        return QStringLiteral("%1 - %2").arg(m_activeTimeSheet->project.name, m_activeTimeSheet->activity.name);
    }

    QString SessionController::activeTimesheetDuration() const
    {
        if (!m_activeTimeSheet)
        {
            return QStringLiteral("00:00:00");
        }
        return FormatHelpers::formatDuration(m_activeTimeSheet->beginAt.secsTo(QDateTime::currentDateTimeUtc()));
    }

    QString SessionController::activeProjectName() const
    {
        return m_activeTimeSheet ? m_activeTimeSheet->project.name : QString{};
    }

    QString SessionController::activeActivityName() const
    {
        return m_activeTimeSheet ? m_activeTimeSheet->activity.name : QString{};
    }

    QString SessionController::activeActivityColor() const
    {
        if (!m_activeTimeSheet)
        {
            return {};
        }
        if (!m_activeTimeSheet->activity.color.isEmpty())
        {
            return m_activeTimeSheet->activity.color;
        }
        if (!m_activeTimeSheet->project.color.isEmpty())
        {
            return m_activeTimeSheet->project.color;
        }
        return QStringLiteral("#888888");
    }

    QDateTime SessionController::activeTimesheetBegin() const
    {
        return m_activeTimeSheet ? m_activeTimeSheet->beginAt.toLocalTime() : QDateTime();
    }

    ActivityListModel *SessionController::activityModel() const
    {
        return m_activityModel.get();
    }

    bool SessionController::isRefreshing() const
    {
        return m_cache->isLoading();
    }

    bool SessionController::isPunchMode() const
    {
        return m_cache->timesheetConfig().trackingMode == KimaiTimeSheetConfig::TrackingMode::Punch;
    }

    int SessionController::lastUsedProjectId() const
    {
        return m_lastUsedProjectId;
    }

    QString SessionController::userTimezone() const
    {
        return m_user.timezone;
    }

    void SessionController::createCustomer(const QString &name, const QString &country, const QString &currency, const QString &timezone)
    {
        KimaiCustomer customer;
        customer.name        = name.trimmed();
        customer.countryKey  = country;
        customer.currencyKey = currency;
        customer.timezone    = timezone;
        // Required by Kimai's customer form: the user's own language (Kimai's default locale "en" if unknown)
        customer.language = m_user.language.isEmpty() ? QStringLiteral("en") : m_user.language;

        m_client->addCustomer(customer)
            .then(this, [this](const KimaiCustomer &created) { emit resourceCreated(ResourceKind::Customer, created.id); })
            .onFailed(this, [this](const KimaiApiError &e) { handleCreateError(e, ResourceKind::Customer); });
    }

    void SessionController::createProject(const QString &name, int customerId)
    {
        KimaiProject project;
        project.name        = name.trimmed();
        project.customer.id = customerId;

        m_client->addProject(project)
            .then(this, [this](const KimaiProject &created) { emit resourceCreated(ResourceKind::Project, created.id); })
            .onFailed(this, [this](const KimaiApiError &e) { handleCreateError(e, ResourceKind::Project); });
    }

    void SessionController::createActivity(const QString &name, int projectId)
    {
        KimaiActivity activity;
        activity.name = name.trimmed();
        if (projectId > 0)
        {
            KimaiProject project;
            project.id       = projectId;
            activity.project = project;
        }

        m_client->addActivity(activity)
            .then(this, [this](const KimaiActivity &created) { emit resourceCreated(ResourceKind::Activity, created.id); })
            .onFailed(this, [this](const KimaiApiError &e) { handleCreateError(e, ResourceKind::Activity); });
    }

    void SessionController::reloadResources()
    {
        if (!m_cache->isLoading())
        {
            m_cache->load(m_client.get());
            emit refreshingChanged();
        }
    }

    namespace
    {
        bool sameName(const QString &a, const QString &b)
        {
            return a.trimmed().compare(b.trimmed(), Qt::CaseInsensitive) == 0;
        }

        QString colorOf(const KimaiActivity &activity)
        {
            if (!activity.color.isEmpty())
            {
                return activity.color;
            }
            if (activity.project.has_value() && !activity.project->color.isEmpty())
            {
                return activity.project->color;
            }
            return QStringLiteral("#888888");
        }

        void sortByName(QVariantList &list)
        {
            std::ranges::sort(list, [](const QVariant &a, const QVariant &b)
                              { return a.toMap().value("name").toString().compare(b.toMap().value("name").toString(), Qt::CaseInsensitive) < 0; });
        }
    } // namespace

    QVariantList SessionController::customerChoices() const
    {
        QVariantList list;
        for (const auto &customer : m_cache->customers())
        {
            const auto projectCount = std::ranges::count_if(m_cache->projects(), [&customer](const KimaiProject &p) { return p.customer.id == customer.id; });
            list.append(QVariantMap{
                {"id",           customer.id                    },
                {"name",         customer.name                  },
                {"projectCount", static_cast<int>(projectCount)}
            });
        }
        sortByName(list);
        return list;
    }

    QVariantList SessionController::projectChoices() const
    {
        QVariantList list;
        for (const auto &project : m_cache->projects())
        {
            list.append(QVariantMap{
                {"id",           project.id           },
                {"name",         project.name         },
                {"customerId",   project.customer.id  },
                {"customerName", project.customer.name},
                {"color",        project.color.isEmpty() ? QStringLiteral("#888888") : project.color}
            });
        }
        sortByName(list);
        return list;
    }

    QVariantList SessionController::globalActivityChoices() const
    {
        // The cache fans global activities out per project: one entry each
        QVariantList list;
        QSet<int>    seen;
        for (const auto &activity : m_cache->activities())
        {
            if (activity.global && !seen.contains(activity.id))
            {
                seen.insert(activity.id);
                list.append(QVariantMap{
                    {"id",    activity.id                                                          },
                    {"name",  activity.name                                                        },
                    {"color", activity.color.isEmpty() ? QStringLiteral("#888888") : activity.color}
                });
            }
        }
        sortByName(list);
        return list;
    }

    QVariantMap SessionController::findCustomer(const QString &name) const
    {
        const auto &customers = m_cache->customers();
        const auto  it        = std::ranges::find_if(customers, [&name](const KimaiCustomer &c) { return sameName(c.name, name); });
        return it == customers.cend() ? QVariantMap{} : QVariantMap{{"id", it->id}, {"name", it->name}};
    }

    QVariantMap SessionController::findProject(const QString &name, int customerId) const
    {
        const auto &projects = m_cache->projects();
        const auto  it       = std::ranges::find_if(projects, [&](const KimaiProject &p) { return p.customer.id == customerId && sameName(p.name, name); });
        return it == projects.cend() ? QVariantMap{} : QVariantMap{{"id", it->id}, {"name", it->name}, {"customerName", it->customer.name}};
    }

    QVariantMap SessionController::findActivity(const QString &name, int projectId) const
    {
        const auto &activities = m_cache->activities();
        const auto  it         = std::ranges::find_if(activities,
                                                      [&](const KimaiActivity &a)
                                                      {
                                                          const auto inScope = projectId > 0 ? (!a.global && a.project && a.project->id == projectId) : a.global;
                                                          return inScope && sameName(a.name, name);
                                                      });
        return it == activities.cend() ? QVariantMap{} : QVariantMap{{"id", it->id}, {"name", it->name}};
    }

    QVariantMap SessionController::pair(int projectId, int activityId) const
    {
        const auto &activities = m_cache->activities();
        const auto  it         = std::ranges::find_if(activities, [&](const KimaiActivity &a) { return a.id == activityId && a.project && a.project->id == projectId; });
        if (it == activities.cend())
        {
            return {};
        }
        return {
            {"projectId",     projectId                                                     },
            {"activityId",    activityId                                                    },
            {"projectName",   it->project->name                                             },
            {"activityName",  it->name                                                      },
            {"activityColor", colorOf(*it)                                                  },
            {"label",         QStringLiteral("%1 - %2").arg(it->project->name, it->name)},
            {"isGlobal",      it->global                                                    }
        };
    }

    bool SessionController::hasTaskManagement() const
    {
        return m_hasTaskManagement;
    }

    TaskModel *SessionController::tasks() const
    {
        return m_tasks.get();
    }

    SessionController::TasksLoad SessionController::tasksLoad() const
    {
        return m_tasksLoad;
    }

    SessionController::NotificationKind SessionController::tasksError() const
    {
        return m_tasksError;
    }

    void SessionController::requestPlugins()
    {
        m_client->requestPlugins()
            .then(this,
                  [this](const KimaiPlugins &plugins)
                  {
                      m_hasTaskManagement = std::ranges::any_of(plugins, [](const KimaiPlugin &p) { return p.apiPlugin == ApiPlugin::TaskManagement; });
                      emit pluginsChanged();
                  })
            .onFailed(this,
                      [this](const KimaiApiError &e)
                      {
                          // Without the list, the plugin features stay hidden: nothing else is affected, no banner
                          spdlog::warn("Failed to fetch plugins: {}", e.what());
                          if (e.kind() == ApiErrorKind::AuthFailed)
                          {
                              emit sessionExpired();
                          }
                      });
    }

    void SessionController::loadTasks()
    {
        if (!m_hasTaskManagement)
        {
            return;
        }

        // The skeleton is for the first load; later loads keep the list shown
        if (m_tasksLoad != TasksLoad::Ready)
        {
            m_tasksLoad = TasksLoad::Loading;
            emit tasksLoadChanged();
        }

        const auto generation = ++m_tasksGeneration;
        m_client->requestTasks(m_user.id)
            .then(this,
                  [this, generation](const KimaiTasks &tasks)
                  {
                      if (generation != m_tasksGeneration)
                      {
                          return;
                      }
                      m_tasks->setTasks(tasks, m_activeTimeSheet ? m_activeTimeSheet->id : 0);
                      m_tasksLoad = TasksLoad::Ready;
                      emit tasksLoadChanged();
                  })
            .onFailed(this,
                      [this, generation](const KimaiApiError &e)
                      {
                          if (generation != m_tasksGeneration)
                          {
                              return;
                          }
                          spdlog::error("Failed to fetch tasks: {}", e.what());
                          if (e.kind() == ApiErrorKind::AuthFailed)
                          {
                              emit sessionExpired();
                              return;
                          }
                          // A list on screen stays (a background reload failed: a warning is enough);
                          // otherwise the tasks pane shows the failure
                          if (m_tasksLoad == TasksLoad::Ready)
                          {
                              emit notificationRequested(NotificationLevel::Warning, notificationKindFromError(e.kind()), e.endpoint());
                              return;
                          }
                          m_tasksError = notificationKindFromError(e.kind());
                          m_tasksLoad  = TasksLoad::Failed;
                          emit tasksLoadChanged();
                      });
    }

    void SessionController::startTask(int taskId)
    {
        const auto *task = m_tasks->findById(taskId);
        if (task == nullptr)
        {
            spdlog::error("Task start ignored: no task with id={}", taskId);
            return;
        }
        const auto title = task->title;

        invalidateSync();
        m_client->startTask(taskId)
            .then(this,
                  [this, taskId, title](const KimaiTimeSheet &started)
                  {
                      emit taskStarted(taskId);

                      // Kimai leaves the description empty: the task title says what the time was spent on
                      if (started.description.isEmpty() && !title.isEmpty())
                      {
                          auto described        = started;
                          described.description = title;
                          m_client->updateTimeSheet(described, m_cache->timesheetConfig().trackingMode)
                              .then(this, [this](const KimaiTimeSheet &) { refreshRecentTimeSheets(); })
                              .onFailed(this, [this](const KimaiApiError &e) { handleApiError(e, "Failed to describe the task's timesheet"); });
                      }
                      refreshActiveTimeSheets();
                      refreshRecentTimeSheets();
                  })
            .onFailed(this, [this, taskId](const KimaiApiError &e) { handleTaskError(e, taskId, "start"); });
    }

    void SessionController::closeTask(int taskId)
    {
        invalidateSync();
        m_client->closeTask(taskId)
            .then(this,
                  [this, taskId](const KimaiTask &)
                  {
                      m_tasks->removeTask(taskId);
                      emit taskClosed(taskId);
                      // Its running timer, if any, was stopped by Kimai
                      refreshActiveTimeSheets();
                      refreshRecentTimeSheets();
                  })
            .onFailed(this, [this, taskId](const KimaiApiError &e) { handleTaskError(e, taskId, "close"); });
    }

    void SessionController::handleTaskError(const KimaiApiError &error, int taskId, const QString &action)
    {
        spdlog::error("Failed to {} task {}: {}", action, taskId, error.what());

        if (error.kind() == ApiErrorKind::AuthFailed)
        {
            emit sessionExpired();
            return;
        }
        emit taskActionFailed(taskId, action, notificationKindFromError(error.kind()), error.serverMessage());
    }

    void SessionController::handleCreateError(const KimaiApiError &error, ResourceKind kind)
    {
        spdlog::error("Failed to create {}: {}", magic_enum::enum_name(kind), error.what());

        if (error.kind() == ApiErrorKind::AuthFailed)
        {
            emit sessionExpired();
            return;
        }

        emit resourceCreateFailed(kind, notificationKindFromError(error.kind()), error.serverMessage(), error.field());
    }

    void SessionController::setStaleAfter(std::chrono::seconds staleAfter)
    {
        m_staleAfter = staleAfter;
    }

    void SessionController::refresh()
    {
        refreshRecentTimeSheets();
        refreshActiveTimeSheets();

        // Coalesce: a load in flight already brings fresh data.
        if (!m_cache->isLoading())
        {
            m_cache->load(m_client.get());
            emit refreshingChanged();
        }
    }

    void SessionController::refreshIfStale()
    {
        if (m_cache->isLoading())
        {
            return;
        }

        // No cache yet means the initial load failed: retrying is the point.
        const auto lastLoadedAt = m_cache->lastLoadedAt();
        if (!lastLoadedAt.isValid() || lastLoadedAt.secsTo(QDateTime::currentDateTimeUtc()) >= m_staleAfter.count())
        {
            refresh();
        }
    }

    void SessionController::refreshRecentTimeSheets()
    {
        m_client->requestTimeSheets()
            .then(this,
                  [this](const KimaiTimeSheets &sheets)
                  {
                      const auto latest = std::ranges::max_element(sheets, {}, &KimaiTimeSheet::beginAt);
                      m_lastUsedProjectId = latest != sheets.cend() ? latest->project.id : 0;
                      m_recentTimeSheets->setTimeSheets(sheets);
                      m_recentRefreshTimer.start();
                  })
            .onFailed(this, [this](const KimaiApiError &e)
                      { handleApiError(e, "Failed to fetch recent timesheets"); });
    }

    void SessionController::refreshActiveTimeSheets()
    {
        m_client->requestActiveTimeSheets()
            .then(this, [this](const KimaiTimeSheets &sheets) { applyActiveTimeSheets(sheets); })
            .onFailed(this, [this](const KimaiApiError &e)
                      { handleApiError(e, "Failed to fetch active timesheets"); });
    }

    bool SessionController::applyActiveTimeSheets(const KimaiTimeSheets &sheets)
    {
        auto next = sheets.isEmpty() ? std::nullopt : std::make_optional(sheets.first());

        // An edit of the running entry from elsewhere counts as a change too
        const auto same = [](const KimaiTimeSheet &a, const KimaiTimeSheet &b)
        {
            return a.id == b.id && a.beginAt == b.beginAt && a.project.id == b.project.id && a.activity.id == b.activity.id &&
                   a.description == b.description;
        };
        const auto changed = m_activeTimeSheet.has_value() != next.has_value() || (next && !same(*m_activeTimeSheet, *next));

        m_activeTimeSheet = std::move(next);

        if (m_activeTimeSheet)
        {
            m_activeDurationTimer.start();
        }
        else
        {
            m_activeDurationTimer.stop();
        }

        if (changed)
        {
            emit activeTimesheetChanged();

            // A timer started or stopped anywhere changes task statuses
            m_tasks->setActiveTimeSheetId(m_activeTimeSheet ? m_activeTimeSheet->id : 0);
            if (m_tasksLoad == TasksLoad::Ready || m_tasksLoad == TasksLoad::Failed)
            {
                loadTasks();
            }
        }
        return changed;
    }

    void SessionController::setSyncInterval(std::chrono::milliseconds interval)
    {
        m_syncTimer.setInterval(interval);
    }

    void SessionController::invalidateSync()
    {
        ++m_syncGeneration;
    }

    void SessionController::syncActiveTimeSheet()
    {
        if (m_syncInFlight)
        {
            return;
        }
        m_syncInFlight = true;

        const auto generation = m_syncGeneration;
        m_client->requestActiveTimeSheets()
            .then(this,
                  [this, generation](const KimaiTimeSheets &sheets)
                  {
                      m_syncInFlight = false;
                      m_syncFailing  = false;

                      // A local action was sent meanwhile: its own refresh has the newer state
                      if (generation != m_syncGeneration)
                      {
                          return;
                      }
                      if (applyActiveTimeSheets(sheets))
                      {
                          spdlog::info("Running timer changed on the server");
                          refreshRecentTimeSheets();
                      }
                  })
            .onFailed(this,
                      [this](const KimaiApiError &e)
                      {
                          m_syncInFlight = false;
                          spdlog::error("Failed to check the running timer: {}", e.what());

                          if (e.kind() == ApiErrorKind::AuthFailed)
                          {
                              emit sessionExpired();
                              return;
                          }
                          // Reported once until a check succeeds: the next ones would repeat the same warning every interval
                          if (!m_syncFailing)
                          {
                              m_syncFailing = true;
                              emit notificationRequested(NotificationLevel::Warning, notificationKindFromError(e.kind()), e.endpoint());
                          }
                      });
    }

    void SessionController::startTimeSheet(int activityId, int projectId)
    {
        if (m_activeTimeSheet)
        {
            spdlog::warn("Start ignored: a timesheet is already active");
            return;
        }
        if (activityId <= 0 || projectId <= 0)
        {
            spdlog::error("Start ignored: invalid activityId={} or projectId={}", activityId, projectId);
            return;
        }

        KimaiTimeSheet ts;
        ts.activity.id = activityId;
        ts.project.id  = projectId;
        ts.beginAt     = QDateTime::currentDateTimeUtc();

        invalidateSync();
        m_client->startTimeSheet(ts, m_cache->timesheetConfig().trackingMode)
            .then(this,
                  [this](const KimaiTimeSheet &)
                  {
                      refreshActiveTimeSheets();
                      refreshRecentTimeSheets();
                  })
            .onFailed(this, [this](const KimaiApiError &e)
                      { handleApiError(e, "Failed to start timesheet"); });
    }

    void SessionController::stopActiveTimeSheet()
    {
        if (!m_activeTimeSheet)
        {
            spdlog::warn("Stop ignored: no active timesheet");
            return;
        }

        invalidateSync();
        m_client->stopTimeSheet(m_activeTimeSheet->id)
            .then(this,
                  [this](const KimaiTimeSheet &)
                  {
                      refreshActiveTimeSheets();
                      refreshRecentTimeSheets();
                  })
            .onFailed(this, [this](const KimaiApiError &e)
                      { handleApiError(e, "Failed to stop timesheet"); });
    }

    void SessionController::stopActiveTimeSheetAt(const QDateTime &end, AwayReason reason)
    {
        if (!m_activeTimeSheet)
        {
            spdlog::warn("Away stop ignored: no active timesheet");
            return;
        }

        const auto onStopped = [this, reason](const KimaiTimeSheet &stopped)
        {
            spdlog::info("Timer stopped while away ({}), ended at {}", magic_enum::enum_name(reason), stopped.endAt.toString(Qt::ISODate));
            refreshActiveTimeSheets();
            refreshRecentTimeSheets();
            emit timeSheetStoppedAway(reason, stopped.endAt.isValid() ? stopped.endAt : QDateTime::currentDateTimeUtc());
        };

        invalidateSync();

        // Kimai sets the end itself in punch mode; an end before the begin (timer started while away) cannot be kept
        if (isPunchMode() || !end.isValid() || end.toUTC() <= m_activeTimeSheet->beginAt)
        {
            m_client->stopTimeSheet(m_activeTimeSheet->id)
                .then(this, onStopped)
                .onFailed(this, [this](const KimaiApiError &e)
                          { handleApiError(e, "Failed to stop timesheet while away"); });
            return;
        }

        KimaiTimeSheet ts = *m_activeTimeSheet;
        ts.endAt          = end.toUTC();
        m_client->updateTimeSheet(ts, m_cache->timesheetConfig().trackingMode)
            .then(this, onStopped)
            .onFailed(this, [this](const KimaiApiError &e)
                      { handleApiError(e, "Failed to stop timesheet at the time the user left"); });
    }

    bool SessionController::updateTimeSheet(int timesheetId, int projectId, int activityId, const QString &description, const QStringList &tags,
                                            const QDateTime &beginAt, const QDateTime &endAt, const QStringList &newTags)
    {
        const auto timeSheet = m_recentTimeSheets->findById(timesheetId);
        if (!timeSheet)
        {
            spdlog::error("Update ignored: no timesheet with id={}", timesheetId);
            return false;
        }
        if (projectId <= 0 || activityId <= 0)
        {
            spdlog::error("Update ignored: invalid projectId={} or activityId={}", projectId, activityId);
            return false;
        }

        KimaiTimeSheet updated = *timeSheet;
        updated.project.id     = projectId;
        updated.activity.id    = activityId;
        updated.description    = description;
        updated.tags           = tags;

        // Invalid datetimes mean "leave unchanged"; a running entry never gets an end here
        if (beginAt.isValid())
        {
            updated.beginAt = beginAt.toUTC();
        }
        if (endAt.isValid() && timeSheet->endAt.isValid())
        {
            updated.endAt = endAt.toUTC();
        }

        if (updated.endAt.isValid() && updated.endAt <= updated.beginAt)
        {
            spdlog::warn("Update ignored: end time must be after begin time");
            return false;
        }

        // Only tags the entry keeps need creating
        QStringList tagsToCreate;
        for (const auto &tag : newTags)
        {
            if (tags.contains(tag))
            {
                tagsToCreate.append(tag);
            }
        }

        createTagsThen(tagsToCreate, timesheetId,
                       [this, updated, timesheetId]()
                       {
                           invalidateSync();
                           m_client->updateTimeSheet(updated, m_cache->timesheetConfig().trackingMode)
                               .then(this,
                                     [this, timesheetId](const KimaiTimeSheet &)
                                     {
                                         refreshActiveTimeSheets();
                                         refreshRecentTimeSheets();
                                         emit timeSheetSaved(timesheetId);
                                     })
                               .onFailed(this, [this, timesheetId](const KimaiApiError &e)
                                         { handleOperationError(e, "Failed to update timesheet", timesheetId, &SessionController::timeSheetSaveFailed); });
                       });
        return true;
    }

    void SessionController::createTagsThen(QStringList names, int timesheetId, std::function<void()> next)
    {
        if (names.isEmpty())
        {
            next();
            return;
        }

        const auto name = names.takeFirst();
        m_client->createTag(name)
            .then(this, [this, names, timesheetId, next]() { createTagsThen(names, timesheetId, next); })
            .onFailed(this,
                      [this, names, timesheetId, next](const KimaiApiError &e)
                      {
                          // 400: the name is already taken (created meanwhile), the entry can use it as is
                          if (e.httpStatus() == 400)
                          {
                              spdlog::warn("Tag creation refused, assuming it exists: {}", e.what());
                              createTagsThen(names, timesheetId, next);
                              return;
                          }
                          handleOperationError(e, "Failed to create tag", timesheetId, &SessionController::timeSheetSaveFailed);
                      });
    }

    bool SessionController::deleteTimeSheet(int timesheetId)
    {
        if (timesheetId <= 0)
        {
            spdlog::error("Delete ignored: invalid timesheetId={}", timesheetId);
            return false;
        }

        invalidateSync();
        m_client->deleteTimeSheet(timesheetId)
            .then(this,
                  [this, timesheetId]()
                  {
                      refreshActiveTimeSheets();
                      refreshRecentTimeSheets();
                      emit timeSheetDeleted(timesheetId);
                  })
            .onFailed(this, [this, timesheetId](const KimaiApiError &e)
                      { handleOperationError(e, "Failed to delete timesheet", timesheetId, &SessionController::timeSheetDeleteFailed); });
        return true;
    }

    void SessionController::requestTagSuggestions(const QString &text)
    {
        m_client->requestTags(text)
            .then(this, [this, text](const QStringList &tags) { emit tagSuggestionsReady(text, tags); })
            .onFailed(this, [this](const KimaiApiError &e)
                      { handleApiError(e, "Failed to fetch tag suggestions"); });
    }

    void SessionController::onCacheLoaded()
    {
        m_activityModel->setActivities(m_cache->activities());
        emit refreshingChanged();
        emit punchModeChanged();
    }

    void SessionController::onCacheLoadFailed()
    {
        emit refreshingChanged();

        // An incomplete cache is useless: surface the failure as a whole.
        if (m_cache->lastError() && m_cache->lastError()->kind() == ApiErrorKind::AuthFailed)
        {
            emit sessionExpired();
            return;
        }

        const auto detail = m_cache->lastError() ? m_cache->lastError()->endpoint() : QString{};

        // A failed refresh keeps the previous complete cache: a warning is enough.
        if (m_cache->hasData())
        {
            emit notificationRequested(NotificationLevel::Warning, NotificationKind::CacheRefreshFailed, detail);
            return;
        }
        emit notificationRequested(NotificationLevel::Error, NotificationKind::CacheLoadFailed, detail);
    }

    void SessionController::handleApiError(const KimaiApiError &error, const char *context)
    {
        spdlog::error("{}: {}", context, error.what());

        if (error.kind() == ApiErrorKind::AuthFailed)
        {
            emit sessionExpired();
            return;
        }

        emit notificationRequested(NotificationLevel::Error, notificationKindFromError(error.kind()), error.endpoint());
    }

    template<typename FailedSignal>
    void SessionController::handleOperationError(const KimaiApiError &error, const char *context, int timesheetId, FailedSignal failed)
    {
        spdlog::error("{}: {}", context, error.what());

        if (error.kind() == ApiErrorKind::AuthFailed)
        {
            emit sessionExpired();
            return;
        }

        // The server message explains refusals such as 400 validation errors
        emit(this->*failed)(timesheetId, notificationKindFromError(error.kind()), error.serverMessage());
    }
} // namespace kemai
