// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "appPreferences.h"

namespace kemai
{
    AppPreferences::AppPreferences(QObject *parent): QObject(parent) {}

    void AppPreferences::setJson(const QJsonObject &json)
    {
        m_json = json;
    }

    QJsonObject AppPreferences::json() const
    {
        return m_json;
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
