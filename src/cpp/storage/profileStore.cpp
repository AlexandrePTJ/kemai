// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "profileStore.h"

// 3rd party headers
#include <spdlog/spdlog.h>

// Qt headers
#include <QVersionNumber>

// Project helpers
#include <misc/customFmt.h>
#include <misc/jsonHelpers.h>
#include <misc/pathHelpers.h>

namespace
{
    /* Changelog
     * =========
     * 2026-02-27    1.0    Initial version
     */
    const auto gProfilesVersion_1_0    = QString("1.0");
    const auto gProfilesCurrentVersion = gProfilesVersion_1_0;

    std::vector<kemai::Profile> loadProfilesFromJson(const QString &storePath)
    {
        std::vector<kemai::Profile> profiles;

        auto json = kemai::JsonHelpers::loadJsonObjectFromFile(storePath);

        // TODO: Handle profiles list version
        // auto currentVersion = QVersionNumber::fromString(gProfilesCurrentVersion);
        // auto version        = QVersionNumber::fromString(json.value("version").toString(gProfilesCurrentVersion));

        auto profileArray = json.value("profiles").toArray();

        for (const auto &profileValue : profileArray)
        {
            profiles.emplace_back(kemai::Profile::fromJson(profileValue.toObject()));
        }

        return profiles;
    }

    void saveProfilesToJson(const QString &storePath, const std::vector<kemai::Profile> &profiles)
    {
        QJsonArray profileArray;
        std::ranges::transform(profiles, std::back_inserter(profileArray), [](const auto &profile)
                               { return profile.toJson(); });

        QJsonObject json;
        json.insert("version", gProfilesCurrentVersion);
        json.insert("profiles", profileArray);

        kemai::JsonHelpers::saveJsonToFile(json, storePath);
    }

    QString getSecretKey(const kemai::ProfileId &profileId)
    {
        return QString("profile/%1/token").arg(profileId.toString());
    }

    QUuid getProfileIdFromSecretKey(QString secretKey)
    {
        secretKey.remove("profile/");
        secretKey.remove("/token");
        return QUuid::fromString(secretKey);
    }

} // namespace

namespace kemai
{
    ProfileStore::ProfileStore(QObject *parent):
    QObject(parent),
    m_secretStorage(this)
    {
        connect(&m_secretStorage, &SecretStorage::secretRead, this, &ProfileStore::onSecretRead);
        connect(&m_secretStorage, &SecretStorage::secretWritten, this, &ProfileStore::onSecretWritten);
        connect(&m_secretStorage, &SecretStorage::secretDeleted, this, &ProfileStore::onSecretDeleted);
        connect(&m_secretStorage, &SecretStorage::readErrorOccurred, this, &ProfileStore::onSecretReadError);
        connect(&m_secretStorage, &SecretStorage::writeErrorOccurred, this, &ProfileStore::onSecretWriteError);
        connect(&m_secretStorage, &SecretStorage::deleteErrorOccurred, this, &ProfileStore::onSecretDeleteError);
    }

    ProfileStore::~ProfileStore() = default;

    void ProfileStore::setStorePath(const QString &storePath)
    {
        m_storePath = storePath;
    }

    void ProfileStore::loadAll()
    {
        try
        {
            m_pendingProfiles = loadProfilesFromJson(m_storePath);

            if (m_pendingProfiles.empty())
            {
                emit profilesLoaded({});
            }

            m_pendingTokenReads = static_cast<int8_t>(m_pendingProfiles.size());
            for (const auto &profile : m_pendingProfiles)
            {
                m_secretStorage.readSecret(getSecretKey(profile.id));
            }
        }
        catch (const std::runtime_error &e)
        {
            spdlog::info("Failed to load profiles from {}: {}", m_storePath, e.what());
        }
    }

    void ProfileStore::save(const Profile &profile)
    {
        if (profile.isValid())
        {
            std::vector<Profile> profiles;
            try
            {
                profiles = loadProfilesFromJson(m_storePath);
            }
            catch (const std::runtime_error &)
            {
                // File doesn't exist yet — start with an empty list and ensure
                // the parent directory exists before writing
                PathHelpers::ensureDirectoryExists(m_storePath);
            }

            auto it = std::ranges::find_if(profiles, [&profile](const Profile &p)
                                           { return p.id == profile.id; });
            if (it != profiles.end())
            {
                *it = profile;
            }
            else
            {
                profiles.push_back(profile);
            }

            saveProfilesToJson(m_storePath, profiles);

            m_secretStorage.writeSecret(getSecretKey(profile.id), profile.token);
        }
    }

    void ProfileStore::remove(const ProfileId &profileId)
    {
        std::vector<Profile> profiles;
        try
        {
            profiles = loadProfilesFromJson(m_storePath);
        }
        catch (const std::runtime_error &)
        {
            // File doesn't exist — nothing to remove
            return;
        }

        std::erase_if(profiles, [&profileId](const Profile &p)
                      { return p.id == profileId; });

        saveProfilesToJson(m_storePath, profiles);

        m_secretStorage.deleteSecret(getSecretKey(profileId));
    }

    void ProfileStore::onSecretRead(const QString &key, const QString &token)
    {
        auto profileId = getProfileIdFromSecretKey(key);
        auto it        = std::ranges::find_if(m_pendingProfiles, [&profileId](const Profile &p)
                                       { return p.id == profileId; });
        if (it != m_pendingProfiles.end())
        {
            it->token = token;
        }

        if (--m_pendingTokenReads <= 0)
        {
            emit profilesLoaded(m_pendingProfiles);
        }
    }

    void ProfileStore::onSecretWritten(const QString &key)
    {
        auto profileId = getProfileIdFromSecretKey(key);
        emit profileSaved(profileId);
    }

    void ProfileStore::onSecretDeleted(const QString &key)
    {
        auto profileId = getProfileIdFromSecretKey(key);
        emit profileRemoved(profileId);
    }

    void ProfileStore::onSecretReadError(const QString &key, const QString &message)
    {
        auto profileId = getProfileIdFromSecretKey(key);
        spdlog::error("Failed to read secret for profile {}: {}", profileId, message);

        if (m_pendingTokenReads > 0)
        {
            if (--m_pendingTokenReads <= 0)
            {
                emit profilesLoaded(m_pendingProfiles);
            }
        }
    }

    void ProfileStore::onSecretWriteError(const QString &key, const QString &message)
    {
        auto profileId = getProfileIdFromSecretKey(key);
        spdlog::error("Failed to write secret for profile {}: {}", profileId, message);
    }

    void ProfileStore::onSecretDeleteError(const QString &key, const QString &message)
    {
        auto profileId = getProfileIdFromSecretKey(key);
        spdlog::error("Failed to delete secret for profile {}: {}", profileId, message);
    }

} // namespace kemai