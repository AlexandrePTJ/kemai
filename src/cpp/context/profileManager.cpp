#include "profileManager.h"

// Project headers
#include <misc/pathHelpers.h>
#include <model/profile.h>
#include <storage/profileStore.h>

namespace kemai
{
    ProfileManager::ProfileManager(QObject *parent):
    QObject(parent),
    m_profileModel(std::make_unique<ProfileModel>()),
    m_profileStore(std::make_unique<ProfileStore>())
    {
        auto profilesPath = PathHelpers::getFilePath(PathHelpers::FilePathType::Profiles);
        m_profileStore->setStorePath(profilesPath);

        connect(m_profileStore.get(), &ProfileStore::profilesLoaded, this, &ProfileManager::onProfilesLoaded);
        connect(m_profileStore.get(), &ProfileStore::profileSaved, this, &ProfileManager::onProfileSaved);
        connect(m_profileStore.get(), &ProfileStore::profileRemoved, this, &ProfileManager::onProfileRemoved);

        m_profileStore->loadAll();
    }

    ProfileManager::~ProfileManager() = default;

    ProfileModel *ProfileManager::profileModel() const
    {
        return m_profileModel.get();
    }

    void ProfileManager::upsert(const ProfileId &profileId, const QString &name, const QString &host, const QString &token)
    {
        Profile profile;
        profile.id    = profileId.isNull() ? ProfileId::createUuid() : profileId;
        profile.name  = name;
        profile.host  = host;
        profile.token = token;

        m_profileStore->save(profile);
    }

    void ProfileManager::remove(const ProfileId &profileId)
    {
        m_profileStore->remove(profileId);
    }

    void ProfileManager::onProfilesLoaded(const std::vector<Profile> &profiles)
    {
        m_profileModel->setProfiles(profiles);
    }

    void ProfileManager::onProfileSaved(ProfileId /*profileId*/)
    {
        m_profileStore->loadAll();
    }

    void ProfileManager::onProfileRemoved(ProfileId /*profileId*/)
    {
        m_profileStore->loadAll();
    }

} // namespace kemai
