// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// Qt headers
#include <QDateTime>
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
        // Window (system-tray): keep running in the tray when the window is closed / minimized (off by default)
        Q_PROPERTY(bool closeToTray READ closeToTray WRITE setCloseToTray NOTIFY closeToTrayChanged)
        Q_PROPERTY(bool minimizeToTray READ minimizeToTray WRITE setMinimizeToTray NOTIFY minimizeToTrayChanged)
        // Updates (about-and-updates): check at start (on by default), the version not to announce at start (empty:
        // none), and when the last check succeeded (invalid: never)
        Q_PROPERTY(bool checkUpdatesAtStart READ checkUpdatesAtStart WRITE setCheckUpdatesAtStart NOTIFY checkUpdatesAtStartChanged)
        Q_PROPERTY(QString ignoredVersion READ ignoredVersion WRITE setIgnoredVersion NOTIFY ignoredVersionChanged)
        Q_PROPERTY(QDateTime lastUpdateCheck READ lastUpdateCheck WRITE setLastUpdateCheck NOTIFY lastUpdateCheckChanged)
        // Profiles import (legacy-profile-import): offer Kemai 0.11's profiles at start, until "Don't ask again"
        Q_PROPERTY(bool legacyImportOffer READ legacyImportOffer WRITE setLegacyImportOffer NOTIFY legacyImportOfferChanged)

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
        int       idleMinutes() const;
        void      setIdleMinutes(int minutes);
        bool      stopOnLock() const;
        void      setStopOnLock(bool stop);
        bool      closeToTray() const;
        void      setCloseToTray(bool enabled);
        bool      minimizeToTray() const;
        void      setMinimizeToTray(bool enabled);
        bool      checkUpdatesAtStart() const;
        void      setCheckUpdatesAtStart(bool check);
        QString   ignoredVersion() const;
        void      setIgnoredVersion(const QString &version);
        QDateTime lastUpdateCheck() const;
        void      setLastUpdateCheck(const QDateTime &when);
        bool      legacyImportOffer() const;
        void      setLegacyImportOffer(bool offer);

    signals:
        // A preference was written: the owner saves the settings
        void changed();
        void stopOnIdleChanged();
        void idleMinutesChanged();
        void stopOnLockChanged();
        void closeToTrayChanged();
        void minimizeToTrayChanged();
        void checkUpdatesAtStartChanged();
        void ignoredVersionChanged();
        void lastUpdateCheckChanged();
        void legacyImportOfferChanged();

    protected:
        QJsonValue value(const QString &key, const QJsonValue &defaultValue) const;
        // Returns false when the value is unchanged (nothing saved)
        bool setValue(const QString &key, const QJsonValue &value);

    private:
        QJsonObject m_json;
    };
} // namespace kemai
