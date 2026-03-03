#pragma once

// STL headers
#include <memory>

// Qt headers
#include <QObject>
#include <QtQmlIntegration/qqmlintegration.h>

// Project headers
#include <data/profile.h>
#include <model/profile.h>

namespace kemai
{
    class ProfileStore;

    class ProfileManager : public QObject
    {
        Q_OBJECT
        QML_ELEMENT

        Q_PROPERTY(ProfileModel *model READ profileModel CONSTANT)

    public:
        explicit ProfileManager(QObject *parent = nullptr);
        ~ProfileManager() override;

        ProfileModel *profileModel() const;

        Q_INVOKABLE void upsert(const ProfileId &profileId, const QString &name, const QString &host, const QString &token);
        Q_INVOKABLE void remove(const ProfileId &profileId);

    private:
        void onProfilesLoaded(const std::vector<Profile> &profiles);
        void onProfileSaved(ProfileId profileId);
        void onProfileRemoved(ProfileId profileId);

        std::unique_ptr<ProfileModel> m_profileModel;
        std::unique_ptr<ProfileStore> m_profileStore;
    };
} // namespace kemai