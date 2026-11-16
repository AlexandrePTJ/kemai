// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "presenceMonitor.h"

// 3rd party headers
#include <spdlog/spdlog.h>

namespace kemai
{
    PresenceMonitor::PresenceMonitor(QObject *parent):
    QObject(parent),
    m_backend(PresenceBackend::create())
    {
        spdlog::info("Presence: idle time {}, session lock {}", m_backend->idleSupported() ? "available" : "unavailable", m_backend->lockSupported() ? "available" : "unavailable");

        m_locked = m_backend->lockedAtStart();

        m_backend->lockChanged = [this](bool locked)
        {
            if (locked == m_locked)
            {
                return;
            }
            m_locked        = locked;
            m_lockChangedAt = QDateTime::currentDateTimeUtc();
            spdlog::info("Session {}", locked ? "locked" : "unlocked");
            emit lockedChanged();
        };
    }

    PresenceMonitor::~PresenceMonitor() = default;

    bool PresenceMonitor::idleSupported() const
    {
        return m_backend->idleSupported();
    }

    bool PresenceMonitor::lockSupported() const
    {
        return m_backend->lockSupported();
    }

    bool PresenceMonitor::isLocked() const
    {
        return m_locked;
    }

    QDateTime PresenceMonitor::lockChangedAt() const
    {
        return m_lockChangedAt;
    }

    int PresenceMonitor::idleSeconds() const
    {
        return m_backend->idleSeconds().value_or(-1);
    }
} // namespace kemai
