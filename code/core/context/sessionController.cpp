// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "sessionController.h"

// spdlog headers
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
    m_cache(std::make_unique<KimaiCache>(this))
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

        KimaiTimeSheet ts = *m_activeTimeSheet;
        ts.endAt          = QDateTime::currentDateTimeUtc();

        invalidateSync();
        m_client->updateTimeSheet(ts, m_cache->timesheetConfig().trackingMode)
            .then(this,
                  [this](const KimaiTimeSheet &)
                  {
                      refreshActiveTimeSheets();
                      refreshRecentTimeSheets();
                  })
            .onFailed(this, [this](const KimaiApiError &e)
                      { handleApiError(e, "Failed to stop timesheet"); });
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
