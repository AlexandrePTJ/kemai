// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "linuxPresenceBackend.h"

// Qt headers
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusObjectPath>
#include <QDBusReply>
#include <QGuiApplication>

// 3rd party headers
#include <spdlog/spdlog.h>

// Project headers
#include <misc/customFmt.h>

#ifdef KEMAI_HAS_XCB_SCREENSAVER
#    include <xcb/screensaver.h>
#    include <xcb/xcb.h>
#endif

namespace
{
    constexpr int DBusTimeoutMs = 1000;

    const auto Login1Service = QStringLiteral("org.freedesktop.login1");
} // namespace

namespace kemai
{
    std::unique_ptr<PresenceBackend> PresenceBackend::create()
    {
        return std::make_unique<LinuxPresenceBackend>();
    }

    LinuxPresenceBackend::LinuxPresenceBackend()
    {
        // Idle: the first source that answers
#ifdef KEMAI_HAS_XCB_SCREENSAVER
        if (QGuiApplication::platformName() == "xcb")
        {
            m_xcbConnection = xcb_connect(nullptr, nullptr);
        }
#endif
        for (const auto source : {IdleSource::Gnome, IdleSource::Kde, IdleSource::X11})
        {
            if (readIdleSeconds(source).has_value())
            {
                m_idleSource = source;
                break;
            }
        }

        // Lock: this process's logind session
        QDBusInterface manager(Login1Service, "/org/freedesktop/login1", "org.freedesktop.login1.Manager", QDBusConnection::systemBus());
        manager.setTimeout(DBusTimeoutMs);
        const QDBusReply<QDBusObjectPath> session = manager.call("GetSession", QStringLiteral("auto"));
        if (session.isValid())
        {
            m_sessionPath   = session.value().path();
            m_lockedAtStart = readLockedHint().value_or(false);
            QDBusConnection::systemBus().connect(Login1Service, m_sessionPath, "org.freedesktop.DBus.Properties", "PropertiesChanged", this,
                                                 SLOT(onSessionPropertiesChanged(QString, QVariantMap, QStringList)));
        }
        else
        {
            spdlog::warn("No logind session for lock events: {}", session.error().message());
        }
    }

    LinuxPresenceBackend::~LinuxPresenceBackend()
    {
#ifdef KEMAI_HAS_XCB_SCREENSAVER
        if (m_xcbConnection != nullptr)
        {
            xcb_disconnect(static_cast<xcb_connection_t *>(m_xcbConnection));
        }
#endif
    }

    bool LinuxPresenceBackend::idleSupported() const
    {
        return m_idleSource != IdleSource::None;
    }

    std::optional<int> LinuxPresenceBackend::idleSeconds() const
    {
        return readIdleSeconds(m_idleSource);
    }

    bool LinuxPresenceBackend::lockSupported() const
    {
        return !m_sessionPath.isEmpty();
    }

    std::optional<int> LinuxPresenceBackend::readIdleSeconds(IdleSource source) const
    {
        switch (source)
        {
        case IdleSource::Gnome: {
            QDBusInterface monitor("org.gnome.Mutter.IdleMonitor", "/org/gnome/Mutter/IdleMonitor/Core", "org.gnome.Mutter.IdleMonitor");
            monitor.setTimeout(DBusTimeoutMs);
            const QDBusReply<qulonglong> ms = monitor.call("GetIdletime");
            return ms.isValid() ? std::optional<int>(static_cast<int>(ms.value() / 1000)) : std::nullopt;
        }
        case IdleSource::Kde: {
            // GNOME also owns this name but answers NotSupported: it is tried after Mutter
            QDBusInterface screenSaver("org.freedesktop.ScreenSaver", "/ScreenSaver", "org.freedesktop.ScreenSaver");
            screenSaver.setTimeout(DBusTimeoutMs);
            const QDBusReply<uint> seconds = screenSaver.call("GetSessionIdleTime");
            return seconds.isValid() ? std::optional<int>(static_cast<int>(seconds.value())) : std::nullopt;
        }
        case IdleSource::X11: {
#ifdef KEMAI_HAS_XCB_SCREENSAVER
            auto *connection = static_cast<xcb_connection_t *>(m_xcbConnection);
            if (connection == nullptr || xcb_connection_has_error(connection) != 0)
            {
                return std::nullopt;
            }
            const auto *screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
            auto       *info   = xcb_screensaver_query_info_reply(connection, xcb_screensaver_query_info(connection, screen->root), nullptr);
            if (info == nullptr)
            {
                return std::nullopt;
            }
            const auto seconds = static_cast<int>(info->ms_since_user_input / 1000);
            free(info);
            return seconds;
#else
            return std::nullopt;
#endif
        }
        case IdleSource::None:
            break;
        }
        return std::nullopt;
    }

    void LinuxPresenceBackend::onSessionPropertiesChanged(const QString &interfaceName, const QVariantMap &changed, const QStringList &invalidated)
    {
        if (interfaceName != "org.freedesktop.login1.Session")
        {
            return;
        }
        if (changed.contains("LockedHint"))
        {
            if (lockChanged)
            {
                lockChanged(changed.value("LockedHint").toBool());
            }
        }
        else if (invalidated.contains("LockedHint"))
        {
            if (const auto locked = readLockedHint(); locked.has_value() && lockChanged)
            {
                lockChanged(*locked);
            }
        }
    }

    bool LinuxPresenceBackend::lockedAtStart() const
    {
        return m_lockedAtStart;
    }

    std::optional<bool> LinuxPresenceBackend::readLockedHint() const
    {
        QDBusInterface session(Login1Service, m_sessionPath, "org.freedesktop.login1.Session", QDBusConnection::systemBus());
        session.setTimeout(DBusTimeoutMs);
        const auto locked = session.property("LockedHint");
        return locked.isValid() ? std::optional<bool>(locked.toBool()) : std::nullopt;
    }
} // namespace kemai
