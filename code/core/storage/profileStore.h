// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// Qt headers
#include <QObject>

// Project headers
#include <data/profile.h>
#include <storage/secretStorage.h>

namespace kemai
{
    class ProfileStore : public QObject
    {
        Q_OBJECT

    public:
        explicit ProfileStore(QObject *parent = nullptr);
        ~ProfileStore() override;

        void setStorePath(const QString &storePath);

        void loadAll();
        void save(const Profile &profile);
        void remove(const ProfileId &profileId);

    signals:
        void profilesLoaded(const std::vector<kemai::Profile> &profiles);
        void profileSaved(kemai::ProfileId profileId);
        void profileRemoved(kemai::ProfileId profileId);

    private:
        void onSecretRead(const QString &key, const QString &token);
        void onSecretWritten(const QString &key);
        void onSecretDeleted(const QString &key);
        void onSecretReadError(const QString &key, const QString &message);
        void onSecretWriteError(const QString &key, const QString &message);
        void onSecretDeleteError(const QString &key, const QString &message);

        SecretStorage m_secretStorage;
        QString       m_storePath;

        // Transient state during loadAll()
        std::vector<Profile> m_pendingProfiles;
        int8_t               m_pendingTokenReads{0};
    };

} // namespace kemai