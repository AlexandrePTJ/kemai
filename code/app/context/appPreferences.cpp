// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "appPreferences.h"

// STL headers
#include <algorithm>

// Project headers
#include <misc/historyPeriods.h>

namespace kemai
{
    AppPreferences::AppPreferences(QObject *parent):
    QObject(parent)
    {
    }

    void AppPreferences::setJson(const QJsonObject &json)
    {
        m_json = json;
    }

    QJsonObject AppPreferences::json() const
    {
        return m_json;
    }

    bool AppPreferences::stopOnIdle() const
    {
        return value("stopOnIdle", false).toBool();
    }

    void AppPreferences::setStopOnIdle(bool stop)
    {
        if (setValue("stopOnIdle", stop))
        {
            emit stopOnIdleChanged();
        }
    }

    int AppPreferences::idleMinutes() const
    {
        return std::clamp(value("idleMinutes", int(DefaultIdleMinutes)).toInt(DefaultIdleMinutes), int(MinIdleMinutes), int(MaxIdleMinutes));
    }

    void AppPreferences::setIdleMinutes(int minutes)
    {
        if (setValue("idleMinutes", std::clamp(minutes, int(MinIdleMinutes), int(MaxIdleMinutes))))
        {
            emit idleMinutesChanged();
        }
    }

    bool AppPreferences::stopOnLock() const
    {
        return value("stopOnLock", false).toBool();
    }

    void AppPreferences::setStopOnLock(bool stop)
    {
        if (setValue("stopOnLock", stop))
        {
            emit stopOnLockChanged();
        }
    }

    bool AppPreferences::closeToTray() const
    {
        return value("closeToTray", false).toBool();
    }

    void AppPreferences::setCloseToTray(bool enabled)
    {
        if (setValue("closeToTray", enabled))
        {
            emit closeToTrayChanged();
        }
    }

    bool AppPreferences::minimizeToTray() const
    {
        return value("minimizeToTray", false).toBool();
    }

    void AppPreferences::setMinimizeToTray(bool enabled)
    {
        if (setValue("minimizeToTray", enabled))
        {
            emit minimizeToTrayChanged();
        }
    }

    bool AppPreferences::checkUpdatesAtStart() const
    {
        return value("checkUpdatesAtStart", true).toBool();
    }

    void AppPreferences::setCheckUpdatesAtStart(bool check)
    {
        if (setValue("checkUpdatesAtStart", check))
        {
            emit checkUpdatesAtStartChanged();
        }
    }

    QString AppPreferences::ignoredVersion() const
    {
        return value("ignoredVersion", QString()).toString();
    }

    void AppPreferences::setIgnoredVersion(const QString &version)
    {
        if (setValue("ignoredVersion", version))
        {
            emit ignoredVersionChanged();
        }
    }

    QDateTime AppPreferences::lastUpdateCheck() const
    {
        return QDateTime::fromString(value("lastUpdateCheck", QString()).toString(), Qt::ISODate);
    }

    void AppPreferences::setLastUpdateCheck(const QDateTime &when)
    {
        if (setValue("lastUpdateCheck", when.toUTC().toString(Qt::ISODate)))
        {
            emit lastUpdateCheckChanged();
        }
    }

    bool AppPreferences::legacyImportOffer() const
    {
        return value("legacyImportOffer", true).toBool();
    }

    void AppPreferences::setLegacyImportOffer(bool offer)
    {
        if (setValue("legacyImportOffer", offer))
        {
            emit legacyImportOfferChanged();
        }
    }

    QString AppPreferences::historyPeriod() const
    {
        return HistoryPeriods::keyOf(HistoryPeriods::fromKey(value("historyPeriod", QString()).toString()));
    }

    void AppPreferences::setHistoryPeriod(const QString &period)
    {
        if (setValue("historyPeriod", HistoryPeriods::keyOf(HistoryPeriods::fromKey(period))))
        {
            emit historyPeriodChanged();
        }
    }

    QJsonValue AppPreferences::value(const QString &key, const QJsonValue &defaultValue) const
    {
        return m_json.contains(key) ? m_json.value(key) : defaultValue;
    }

    bool AppPreferences::setValue(const QString &key, const QJsonValue &value)
    {
        if (m_json.value(key) == value)
        {
            return false;
        }
        m_json.insert(key, value);
        emit changed();
        return true;
    }
} // namespace kemai
