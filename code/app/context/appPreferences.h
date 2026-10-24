// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// Qt headers
#include <QJsonObject>
#include <QObject>
#include <QtQmlIntegration/qqmlintegration.h>

namespace kemai
{
    /*!
     * Per-computer preferences shown in the settings dialog. Each feature adds a Q_PROPERTY reading and writing
     * one key through value()/setValue() (with its default); every write is saved at once through `changed`.
     * The whole JSON object is kept, so keys of a newer Kemai survive.
     */
    class AppPreferences : public QObject
    {
        Q_OBJECT
        QML_ELEMENT
        QML_UNCREATABLE("Owned by KemaiContext")

        // Inactivity (inactivity-stop)
        Q_PROPERTY(bool stopOnIdle READ stopOnIdle WRITE setStopOnIdle NOTIFY stopOnIdleChanged)
        Q_PROPERTY(int idleMinutes READ idleMinutes WRITE setIdleMinutes NOTIFY idleMinutesChanged)
        Q_PROPERTY(bool stopOnLock READ stopOnLock WRITE setStopOnLock NOTIFY stopOnLockChanged)

    public:
        explicit AppPreferences(QObject *parent = nullptr);

        // An enum so QML reads them too (AppPreferences.MinIdleMinutes)
        enum IdleLimits
        {
            MinIdleMinutes     = 1,
            MaxIdleMinutes     = 120,
            DefaultIdleMinutes = 10
        };
        Q_ENUM(IdleLimits)

        void        setJson(const QJsonObject &json);
        QJsonObject json() const;

        bool stopOnIdle() const;
        void setStopOnIdle(bool stop);
        // Clamped to MinIdleMinutes..MaxIdleMinutes
        int  idleMinutes() const;
        void setIdleMinutes(int minutes);
        bool stopOnLock() const;
        void setStopOnLock(bool stop);

    signals:
        // A preference was written: the owner saves the settings
        void changed();
        void stopOnIdleChanged();
        void idleMinutesChanged();
        void stopOnLockChanged();

    protected:
        QJsonValue value(const QString &key, const QJsonValue &defaultValue) const;
        // Returns false when the value is unchanged (nothing saved)
        bool       setValue(const QString &key, const QJsonValue &value);

    private:
        QJsonObject m_json;
    };
} // namespace kemai
