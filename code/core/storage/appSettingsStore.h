// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// Qt headers
#include <QString>

// Project headers
#include <data/appSettings.h>
#include <storage/secretStorage.h>

namespace kemai
{
    class AppSettingsStore : public QObject
    {
        Q_OBJECT

    public:
        explicit AppSettingsStore(QObject *parent = nullptr);
        ~AppSettingsStore() = default;

        void setStorePath(const QString &storePath);

        void load();
        void save(const AppSettings &settings);

    signals:
        void settingsLoaded(const AppSettings &settings);

    private:
        void onSecretRead(const QString &key, const QString &value);
        void onSecretReadError(const QString &key, const QString &message);

        SecretStorage m_secretStorage;
        QString       m_storePath;

        // Transient state during load()
        AppSettings m_pendingSettings;
    };

} // namespace kemai
