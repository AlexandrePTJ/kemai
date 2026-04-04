// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "sessionContext.h"

// Qt headers
#include <QTimer>

// spdlog headers
#include <spdlog/spdlog.h>

namespace kemai
{
    SessionContext::SessionContext(std::shared_ptr<KimaiClient> client, const KimaiUser &user, QObject *parent):
    QObject(parent),
    m_client(std::move(client)),
    m_user(user),
    m_recentTimeSheets(std::make_unique<TimesheetModel>(this))
    {
        m_activeEntryTimer.setInterval(std::chrono::minutes(1));
        connect(&m_activeEntryTimer, &QTimer::timeout, m_recentTimeSheets.get(), &TimesheetModel::refreshActiveDurations);

        refreshRecentTimeSheets();
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

    void SessionContext::refreshRecentTimeSheets()
    {
        m_client->requestRecentTimeSheets(std::nullopt, 30)
            .then(this,
                  [this](const KimaiTimeSheets &sheets)
                  {
                      m_recentTimeSheets->setTimeSheets(sheets);

                      const auto hasActive = std::any_of(sheets.begin(), sheets.end(), [](const KimaiTimeSheet &ts)
                                                         { return !ts.endAt.isValid(); });
                      if (hasActive)
                      {
                          m_activeEntryTimer.start();
                      }
                      else
                      {
                          m_activeEntryTimer.stop();
                      }
                  })
            .onFailed(this, [](const std::exception &e)
                      { spdlog::error("Failed to fetch recent timesheets: {}", e.what()); });
    }

} // namespace kemai
