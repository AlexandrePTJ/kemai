// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// Qt headers
#include <QJsonObject>
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

        // Parses a profiles file content; throws std::runtime_error on a missing/invalid
        // version or a version from a newer major (see the versioning policy in the .cpp).
        static std::vector<Profile> profilesFromJson(const QJsonObject &json);

        void setStorePath(const QString &storePath);

        void loadAll();
        void save(const Profile &profile);
        void remove(const ProfileId &profileId);

    signals:
        void profilesLoaded(const std::vector<kemai::Profile> &profiles);
        void profileSaved(kemai::ProfileId profileId);
        // The profile could not be saved (profiles file unreadable or not writable, keychain refused the token);
        // message is technical, for the log and the UI to word
        void profileSaveFailed(kemai::ProfileId profileId, const QString &message);
        void profileRemoved(kemai::ProfileId profileId);

    private:
        void onSecretRead(const QString &key, const QString &token);
        void onSecretWritten(const QString &key);
        void onSecretDeleted(const QString &key);
        void onSecretReadError(const QString &key, const QString &message);
        void onSecretWriteError(const QString &key, const QString &message);
        void onSecretDeleteError(const QString &key, const QString &message);

        void removeFromFile(const ProfileId &profileId);

        SecretStorage m_secretStorage;
        QString       m_storePath;

        // Profiles added by a save() whose token is not in the keychain yet: removed again if the keychain refuses it
        std::vector<ProfileId> m_addedProfiles;

        // Transient state during loadAll()
        std::vector<Profile> m_pendingProfiles;
        int8_t               m_pendingTokenReads{0};
    };

} // namespace kemai