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

        // Asynchronous: the auto-login token comes from the keychain; answered by settingsLoaded (not on a missing/unreadable file)
        void load();
        // Synchronous file read, without the token: defaults when the file is missing or unreadable
        AppSettings readFile() const;
        // False when an existing settings file cannot be read (corrupt, newer major): save() then leaves it untouched,
        // settings only live for this session
        bool canSave() const;
        // The token is written to the keychain only when present: settings read by readFile() carry none
        void save(const AppSettings &settings);

        // Throws std::runtime_error on a missing/invalid version or one written by a newer major
        static AppSettings settingsFromJson(const QJsonObject &json);

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
