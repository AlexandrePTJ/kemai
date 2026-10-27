// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "profileStore.h"

// 3rd party headers
#include <spdlog/spdlog.h>

// Qt headers
#include <QFileInfo>
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
     *
     * Versioning policy
     * - minor bump: additive change only; an older Kemai reads the file and ignores unknown fields.
     * - major bump: breaking change; add a migration from the previous major in profilesFromJson().
     *   An older Kemai refuses to read or overwrite a file with a newer major.
     */
    const auto gProfilesCurrentVersion = QVersionNumber(1, 0);

    std::vector<kemai::Profile> loadProfilesFromJson(const QString &storePath)
    {
        return kemai::ProfileStore::profilesFromJson(kemai::JsonHelpers::loadJsonObjectFromFile(storePath));
    }

    void saveProfilesToJson(const QString &storePath, const std::vector<kemai::Profile> &profiles)
    {
        QJsonArray profileArray;
        std::ranges::transform(profiles, std::back_inserter(profileArray), [](const auto &profile)
                               { return profile.toJson(); });

        QJsonObject json;
        json.insert("version", gProfilesCurrentVersion.toString());
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

    std::vector<Profile> ProfileStore::profilesFromJson(const QJsonObject &json)
    {
        const auto versionString = json.value("version").toString();
        const auto version       = QVersionNumber::fromString(versionString);
        if (version.isNull())
        {
            throw std::runtime_error(fmt::format("Missing or invalid profiles version '{}'", versionString));
        }

        if (version.majorVersion() > gProfilesCurrentVersion.majorVersion())
        {
            throw std::runtime_error(fmt::format("Profiles version {} was written by a newer Kemai (supports up to {}.x)", versionString, gProfilesCurrentVersion.majorVersion()));
        }

        // Migrations from older majors go here, oldest first (none yet: 1.0 is the first format).

        std::vector<Profile> profiles;
        for (const auto &profileValue : json.value("profiles").toArray())
        {
            profiles.emplace_back(Profile::fromJson(profileValue.toObject()));
        }
        return profiles;
    }

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
            if (QFileInfo::exists(m_storePath))
            {
                try
                {
                    profiles = loadProfilesFromJson(m_storePath);
                }
                catch (const std::runtime_error &e)
                {
                    // Unreadable, corrupt or newer-format file: overwriting it would lose every other profile
                    spdlog::error("Profile not saved, {} left untouched: {}", m_storePath, e.what());
                    emit profileSaveFailed(profile.id, QString::fromUtf8(e.what()));
                    return;
                }
            }
            else
            {
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
                m_addedProfiles.push_back(profile.id);
            }

            try
            {
                saveProfilesToJson(m_storePath, profiles);
            }
            catch (const std::runtime_error &e)
            {
                spdlog::error("Profile not saved: {}", e.what());
                std::erase(m_addedProfiles, profile.id);
                emit profileSaveFailed(profile.id, QString::fromUtf8(e.what()));
                return;
            }

            m_secretStorage.writeSecret(getSecretKey(profile.id), profile.token);
        }
    }

    void ProfileStore::remove(const ProfileId &profileId)
    {
        removeFromFile(profileId);
        m_secretStorage.deleteSecret(getSecretKey(profileId));
    }

    void ProfileStore::removeFromFile(const ProfileId &profileId)
    {
        std::vector<Profile> profiles;
        try
        {
            profiles = loadProfilesFromJson(m_storePath);
        }
        catch (const std::runtime_error &e)
        {
            // Missing file: nothing to remove. Unreadable or newer-format file: keep it as is.
            spdlog::debug("Profile not removed from {}: {}", m_storePath, e.what());
            return;
        }

        std::erase_if(profiles, [&profileId](const Profile &p)
                      { return p.id == profileId; });

        saveProfilesToJson(m_storePath, profiles);
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
        std::erase(m_addedProfiles, profileId);
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

        // A new profile without its token cannot log in: remove it again. An existing one keeps its previous token.
        if (std::erase(m_addedProfiles, profileId) > 0)
        {
            try
            {
                removeFromFile(profileId);
            }
            catch (const std::runtime_error &e)
            {
                spdlog::error("Profile {} without token left in {}: {}", profileId, m_storePath, e.what());
            }
        }
        emit profileSaveFailed(profileId, message);
    }

    void ProfileStore::onSecretDeleteError(const QString &key, const QString &message)
    {
        auto profileId = getProfileIdFromSecretKey(key);
        spdlog::error("Failed to delete secret for profile {}: {}", profileId, message);
    }

} // namespace kemai