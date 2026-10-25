// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// Qt headers
#include <QObject>
#include <QStringList>
#include <QVariantMap>

// Project headers
#include <presenceMonitor.h>

namespace kemai
{
    /*!
     * Linux: idle time from GNOME (Mutter IdleMonitor) or KDE (org.freedesktop.ScreenSaver) over D-Bus, else the
     * X11 screensaver extension in an X11 session; lock from logind's LockedHint (any desktop with logind).
     */
    class LinuxPresenceBackend : public QObject, public PresenceBackend
    {
        Q_OBJECT

    public:
        LinuxPresenceBackend();
        ~LinuxPresenceBackend() override;

        bool               idleSupported() const override;
        std::optional<int> idleSeconds() const override;
        bool               lockSupported() const override;
        bool               lockedAtStart() const override;

    private slots:
        void onSessionPropertiesChanged(const QString &interfaceName, const QVariantMap &changed, const QStringList &invalidated);

    private:
        enum class IdleSource
        {
            None,
            Gnome,
            Kde,
            X11
        };

        std::optional<int> readIdleSeconds(IdleSource source) const;
        std::optional<bool> readLockedHint() const;

        IdleSource m_idleSource = IdleSource::None;
        QString    m_sessionPath;
        bool       m_lockedAtStart = false;
        void      *m_xcbConnection = nullptr;
    };
} // namespace kemai
