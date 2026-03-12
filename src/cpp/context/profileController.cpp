// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "profileController.h"

// Project headers
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
        connect(m_profileStore.get(), &ProfileStore::profileRemoved, this, &ProfileController::onProfileRemoved);

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

    void ProfileController::onProfilesLoaded(const std::vector<Profile> &profiles)
    {
        m_profileModel->setProfiles(profiles);
    }

    void ProfileController::onProfileSaved(ProfileId /*profileId*/)
    {
        m_profileStore->loadAll();
    }

    void ProfileController::onProfileRemoved(ProfileId /*profileId*/)
    {
        m_profileStore->loadAll();
    }

} // namespace kemai
