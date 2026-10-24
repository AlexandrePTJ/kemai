// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// STL headers
#include <functional>
#include <memory>
#include <optional>

// Qt headers
#include <QDateTime>
#include <QObject>
#include <QtQmlIntegration/qqmlintegration.h>

namespace kemai
{
    /*!
     * Platform source of user presence: idle time (no keyboard/mouse input) and session lock events.
     * One implementation per OS; each says what it supports on the current desktop.
     */
    class PresenceBackend
    {
    public:
        virtual ~PresenceBackend() = default;

        virtual bool               idleSupported() const = 0;
        // Seconds since the last input; nullopt when it cannot be read right now
        virtual std::optional<int> idleSeconds() const   = 0;
        virtual bool               lockSupported() const = 0;
        // Session already locked when the backend was created
        virtual bool               lockedAtStart() const { return false; }

        // Called on the main thread when the session gets locked (true) or unlocked (false)
        std::function<void(bool)> lockChanged;

        static std::unique_ptr<PresenceBackend> create();
    };

    /*!
     * QML face of the platform presence backend. Idle time is read on demand (the caller polls it only
     * while it matters); lock changes are pushed.
     */
    class PresenceMonitor : public QObject
    {
        Q_OBJECT
        QML_ELEMENT

        Q_PROPERTY(bool idleSupported READ idleSupported CONSTANT)
        Q_PROPERTY(bool lockSupported READ lockSupported CONSTANT)
        Q_PROPERTY(bool locked READ isLocked NOTIFY lockedChanged)
        // When `locked` last changed
        Q_PROPERTY(QDateTime lockChangedAt READ lockChangedAt NOTIFY lockedChanged)

    public:
        explicit PresenceMonitor(QObject *parent = nullptr);
        ~PresenceMonitor() override;

        bool      idleSupported() const;
        bool      lockSupported() const;
        bool      isLocked() const;
        QDateTime lockChangedAt() const;

        // Seconds without input, -1 when unknown
        Q_INVOKABLE int idleSeconds() const;

    signals:
        void lockedChanged();

    private:
        std::unique_ptr<PresenceBackend> m_backend;
        bool                             m_locked = false;
        QDateTime                        m_lockChangedAt;
    };
} // namespace kemai
