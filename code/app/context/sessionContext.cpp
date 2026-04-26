// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "sessionContext.h"

// spdlog headers
#include <spdlog/spdlog.h>

// Project headers
#include <misc/customFmt.h>
#include <misc/formatHelpers.h>

namespace kemai
{
    SessionContext::SessionContext(std::shared_ptr<KimaiClient> client, const KimaiUser &user, QObject *parent):
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
        connect(&m_activeDurationTimer, &QTimer::timeout, this, &SessionContext::activeTimesheetChanged);

        connect(m_cache.get(), &KimaiCache::loaded, this, &SessionContext::onCacheLoaded);

        refreshRecentTimeSheets();
        refreshActiveTimeSheets();
        m_cache->load(m_client.get());
    }

    SessionContext::~SessionContext() = default;

    QString SessionContext::username() const
    {
        return m_user.username;
    }

    TimesheetModel *SessionContext::recentTimeSheets() const
    {
        return m_recentTimeSheets.get();
    }

    bool SessionContext::hasActiveTimesheet() const
    {
        return m_activeTimeSheet.has_value();
    }

    QString SessionContext::activeTimesheetLabel() const
    {
        if (!m_activeTimeSheet)
        {
            return {};
        }
        return QStringLiteral("%1 - %2").arg(m_activeTimeSheet->project.name, m_activeTimeSheet->activity.name);
    }

    QString SessionContext::activeTimesheetDuration() const
    {
        if (!m_activeTimeSheet)
        {
            return QStringLiteral("00:00:00");
        }
        return FormatHelpers::formatDuration(m_activeTimeSheet->beginAt.secsTo(QDateTime::currentDateTimeUtc()));
    }

    ActivityListModel *SessionContext::activityModel() const
    {
        return m_activityModel.get();
    }

    void SessionContext::refreshRecentTimeSheets()
    {
        m_client->requestTimeSheets()
            .then(this,
                  [this](const KimaiTimeSheets &sheets)
                  {
                      m_recentTimeSheets->setTimeSheets(sheets);
                      m_recentRefreshTimer.start();
                  })
            .onFailed(this, [](const std::exception &e)
                      { spdlog::error("Failed to fetch recent timesheets: {}", e.what()); });
    }

    void SessionContext::refreshActiveTimeSheets()
    {
        m_client->requestActiveTimeSheets()
            .then(this,
                  [this](const KimaiTimeSheets &sheets)
                  {
                      const auto hadActive = m_activeTimeSheet.has_value();
                      m_activeTimeSheet    = sheets.isEmpty() ? std::nullopt : std::make_optional(sheets.first());

                      if (m_activeTimeSheet)
                      {
                          m_activeDurationTimer.start();
                      }
                      else
                      {
                          m_activeDurationTimer.stop();
                      }

                      if (hadActive != m_activeTimeSheet.has_value())
                      {
                          emit activeTimesheetChanged();
                      }
                  })
            .onFailed(this, [](const std::exception &e)
                      { spdlog::error("Failed to fetch active timesheets: {}", e.what()); });
    }

    void SessionContext::startTimeSheet(int activityId, int projectId)
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

        m_client->startTimeSheet(ts, m_cache->timesheetConfig().trackingMode)
            .then(this,
                  [this](const KimaiTimeSheet &)
                  {
                      refreshActiveTimeSheets();
                      refreshRecentTimeSheets();
                  })
            .onFailed(this, [](const std::exception &e)
                      { spdlog::error("Failed to start timesheet: {}", e.what()); });
    }

    void SessionContext::stopActiveTimeSheet()
    {
        if (!m_activeTimeSheet)
        {
            spdlog::warn("Stop ignored: no active timesheet");
            return;
        }

        KimaiTimeSheet ts = *m_activeTimeSheet;
        ts.endAt          = QDateTime::currentDateTimeUtc();

        m_client->updateTimeSheet(ts, m_cache->timesheetConfig().trackingMode)
            .then(this,
                  [this](const KimaiTimeSheet &)
                  {
                      refreshActiveTimeSheets();
                      refreshRecentTimeSheets();
                  })
            .onFailed(this, [](const std::exception &e)
                      { spdlog::error("Failed to stop timesheet: {}", e.what()); });
    }

    void SessionContext::onCacheLoaded()
    {
        m_activityModel->setActivities(m_cache->activities());
    }

} // namespace kemai
