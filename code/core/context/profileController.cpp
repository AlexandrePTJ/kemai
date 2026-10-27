// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "profileController.h"

// 3rd party headers
#include <spdlog/spdlog.h>

// Qt headers
#include <QFileInfo>

// Project headers
#include <client/kimaiClient.h>
#include <misc/customFmt.h>
#include <misc/pathHelpers.h>
#include <model/profile.h>
#include <storage/profileStore.h>

namespace kemai
{
    ProfileController::ProfileController(QObject *parent):
    QObject(parent),
    m_profileModel(std::make_unique<ProfileModel>()),
    m_profileStore(std::make_unique<ProfileStore>())
    {
        auto profilesPath = PathHelpers::getFilePath(PathHelpers::FilePathType::Profiles);
        m_profileStore->setStorePath(profilesPath);

        connect(m_profileStore.get(), &ProfileStore::profilesLoaded, this, &ProfileController::onProfilesLoaded);
        connect(m_profileStore.get(), &ProfileStore::profileSaved, this, &ProfileController::onProfileSaved);
        connect(m_profileStore.get(), &ProfileStore::profileSaveFailed, this, &ProfileController::onProfileSaveFailed);
        connect(m_profileStore.get(), &ProfileStore::profileRemoved, this, &ProfileController::onProfileRemoved);

        m_legacyProfiles = LegacySettingsReader::readProfiles(PathHelpers::getFilePath(PathHelpers::FilePathType::LegacySettings));
        // Without a profiles file, loadAll() reports nothing: there is no profile. Otherwise wait for them.
        m_profilesKnown = !QFileInfo::exists(profilesPath);
        updateLegacyCandidates();

        m_profileStore->loadAll();
    }

    ProfileController::~ProfileController() = default;

    ProfileModel *ProfileController::profileModel() const
    {
        return m_profileModel.get();
    }

    void ProfileController::upsert(const ProfileId &profileId, const QString &name, const QString &host, const QString &token)
    {
        Profile profile;
        profile.id    = profileId.isNull() ? ProfileId::createUuid() : profileId;
        profile.name  = name;
        profile.host  = host;
        profile.token = token;

        m_profileStore->save(profile);
    }

    void ProfileController::remove(const ProfileId &profileId)
    {
        m_profileStore->remove(profileId);
    }

    std::vector<LegacyCandidate> ProfileController::legacyCandidatesOf(const std::vector<LegacyProfile> &legacyProfiles, const std::vector<Profile> &profiles)
    {
        // Names taken so far, with their host: existing profiles, then the candidates to import
        std::vector<std::pair<QString, QString>> takenNames;
        for (const auto &profile : profiles)
        {
            takenNames.emplace_back(profile.name, KimaiClient::normalizedHost(profile.host));
        }

        std::vector<LegacyCandidate> candidates;
        for (const auto &legacyProfile : legacyProfiles)
        {
            const auto host = KimaiClient::normalizedHost(legacyProfile.host);

            LegacyCandidate candidate{.profile = legacyProfile, .finalName = legacyProfile.name};
            const auto      alreadyImported = std::ranges::any_of(profiles, [&legacyProfile, &host](const Profile &profile)
                                                                  { return (!legacyProfile.id.isNull() && profile.id == legacyProfile.id) ||
                                                                           (profile.name == legacyProfile.name && KimaiClient::normalizedHost(profile.host) == host); });
            if (alreadyImported)
            {
                candidate.state = LegacyCandidate::State::AlreadyImported;
            }
            else if (legacyProfile.apiToken.isEmpty())
            {
                candidate.state = LegacyCandidate::State::NoToken;
            }
            else
            {
                const auto nameTaken = std::ranges::any_of(takenNames, [&legacyProfile, &host](const auto &taken)
                                                           { return taken.first == legacyProfile.name && taken.second != host; });
                if (nameTaken)
                {
                    candidate.finalName = legacyProfile.name + " (0.11)";
                }
                takenNames.emplace_back(candidate.finalName, host);
            }
            candidates.push_back(candidate);
        }
        return candidates;
    }

    const std::vector<LegacyCandidate> &ProfileController::legacyCandidates() const
    {
        return m_legacyCandidates;
    }

    bool ProfileController::hasLegacyCandidates() const
    {
        return m_profilesKnown && std::ranges::any_of(m_legacyCandidates, [](const LegacyCandidate &candidate)
                                                      { return candidate.state == LegacyCandidate::State::Ready; });
    }

    bool ProfileController::legacyCandidatesKnown() const
    {
        return m_profilesKnown;
    }

    QString ProfileController::legacyLastProfileName() const
    {
        auto it = std::ranges::find_if(m_legacyCandidates, [](const LegacyCandidate &candidate)
                                       { return candidate.profile.lastConnected; });
        if (it == m_legacyCandidates.end() || it->state == LegacyCandidate::State::NoToken)
        {
            return {};
        }
        return it->finalName;
    }

    void ProfileController::importLegacyProfiles()
    {
        if (m_importing)
        {
            return;
        }

        m_importQueue.clear();
        m_importedIds.clear();
        m_importFailures.clear();
        for (const auto &candidate : m_legacyCandidates)
        {
            if (candidate.state == LegacyCandidate::State::Ready)
            {
                const auto &legacyProfile = candidate.profile;
                // The 0.11 id, so that the import is recognized afterwards
                m_importQueue.push_back({.id    = legacyProfile.id.isNull() ? ProfileId::createUuid() : legacyProfile.id,
                                         .name  = candidate.finalName,
                                         .host  = legacyProfile.host,
                                         .token = legacyProfile.apiToken});
            }
        }

        m_importing = true;
        importNextLegacyProfile();
    }

    void ProfileController::importNextLegacyProfile()
    {
        if (m_importQueue.empty())
        {
            m_importing     = false;
            m_importCurrent = {};
            spdlog::info("Kemai 0.11 profiles imported: {}, failed: {}", m_importedIds.size(), m_importFailures.size());

            // Imported now, even before the profiles are reloaded: a Retry must not save them again
            for (auto &candidate : m_legacyCandidates)
            {
                if (m_importedIds.contains(candidate.profile.id))
                {
                    candidate.state = LegacyCandidate::State::AlreadyImported;
                }
            }
            emit legacyCandidatesChanged();

            m_profileStore->loadAll();
            emit legacyImportFinished(m_importedIds, m_importFailures);
            return;
        }

        const auto profile = m_importQueue.front();
        m_importQueue.erase(m_importQueue.begin());
        m_importCurrent = profile.id;
        // Answered by profileSaved or profileSaveFailed, possibly before save() returns
        m_profileStore->save(profile);
    }

    void ProfileController::onProfilesLoaded(const std::vector<Profile> &profiles)
    {
        m_profiles      = profiles;
        m_profilesKnown = true;
        m_profileModel->setProfiles(profiles);
        updateLegacyCandidates();
    }

    void ProfileController::onProfileSaved(ProfileId profileId)
    {
        if (m_importing)
        {
            // Reloaded once at the end: reloads overlapping the next saves would mix their token reads
            if (profileId == m_importCurrent)
            {
                m_importedIds.push_back(profileId);
                importNextLegacyProfile();
            }
            return;
        }
        m_profileStore->loadAll();
    }

    void ProfileController::onProfileSaveFailed(ProfileId profileId, const QString &message)
    {
        // Logged by the store. Outside an import, the profile manager has no error channel yet: the profile simply
        // does not appear
        if (m_importing && profileId == m_importCurrent)
        {
            m_importFailures.insert(profileId, message);
            importNextLegacyProfile();
        }
    }

    void ProfileController::onProfileRemoved(ProfileId /*profileId*/)
    {
        m_profileStore->loadAll();
    }

    void ProfileController::updateLegacyCandidates()
    {
        m_legacyCandidates = legacyCandidatesOf(m_legacyProfiles, m_profiles);
        emit legacyCandidatesChanged();
    }

} // namespace kemai
