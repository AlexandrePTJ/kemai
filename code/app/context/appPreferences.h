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

    public:
        explicit AppPreferences(QObject *parent = nullptr);

        void        setJson(const QJsonObject &json);
        QJsonObject json() const;

    signals:
        // A preference was written: the owner saves the settings
        void changed();

    protected:
        QJsonValue value(const QString &key, const QJsonValue &defaultValue) const;
        // Returns false when the value is unchanged (nothing saved)
        bool       setValue(const QString &key, const QJsonValue &value);

    private:
        QJsonObject m_json;
    };
} // namespace kemai
