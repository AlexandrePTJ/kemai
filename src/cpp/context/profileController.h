// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// STL headers
#include <memory>

// Qt headers
#include <QObject>

// Project headers
#include <data/profile.h>
#include <model/profile.h>

namespace kemai
{
    class ProfileStore;

    class ProfileController : public QObject
    {
        Q_OBJECT

    public:
        explicit ProfileController(QObject *parent = nullptr);
        ~ProfileController() override;

        ProfileModel *profileModel() const;

        void upsert(const ProfileId &profileId, const QString &name, const QString &host, const QString &token);
        void remove(const ProfileId &profileId);

    private:
        void onProfilesLoaded(const std::vector<Profile> &profiles);
        void onProfileSaved(ProfileId profileId);
        void onProfileRemoved(ProfileId profileId);

        std::unique_ptr<ProfileModel> m_profileModel;
        std::unique_ptr<ProfileStore> m_profileStore;
    };
} // namespace kemai
