// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// STL headers
#include <memory>
#include <vector>

// Qt headers
#include <QList>
#include <QMap>
#include <QObject>

// Project headers
#include <data/profile.h>
#include <model/profile.h>
#include <storage/legacySettingsReader.h>

namespace kemai
{
    class ProfileStore;

    // A Kemai 0.11 profile, as offered for import
    struct LegacyCandidate
    {
        enum class State
        {
            Ready,          // has an API token: imported by importLegacyProfiles()
            NoToken,        // only 0.11's legacy API password: never imported
            AlreadyImported // a profile with the same id, or the same name and host, exists
        };

        LegacyProfile profile;
        State         state{State::Ready};
        QString       finalName; // the name it gets: "<name> (0.11)" when another server's profile has the name
    };

    class ProfileController : public QObject
    {
        Q_OBJECT

    public:
        explicit ProfileController(QObject *parent = nullptr);
        ~ProfileController() override;

        ProfileModel *profileModel() const;

        void upsert(const ProfileId &profileId, const QString &name, const QString &host, const QString &token);
        void remove(const ProfileId &profileId);

        // Kemai 0.11's profiles against the existing ones (names compared case-sensitively, hosts normalized)
        static std::vector<LegacyCandidate> legacyCandidatesOf(const std::vector<LegacyProfile> &legacyProfiles, const std::vector<Profile> &profiles);

        const std::vector<LegacyCandidate> &legacyCandidates() const;
        // At least one Ready candidate, once the existing profiles are known (loaded, or no profiles file): profiles
        // without token alone offer nothing
        bool hasLegacyCandidates() const;
        // The existing profiles are known (loaded, or no profiles file): until then every candidate looks new
        bool legacyCandidatesKnown() const;
        // Final name of the profile 0.11 last connected with, empty when it has none or it has no token
        QString legacyLastProfileName() const;
        // Saves every Ready candidate, one after the other, with its 0.11 id; answered by legacyImportFinished.
        // Called again (Retry), it saves only what is still Ready. Ignored while an import runs.
        void importLegacyProfiles();

    signals:
        void legacyCandidatesChanged();
        // failures: profile id -> technical reason, worded by the UI
        void legacyImportFinished(const QList<kemai::ProfileId> &importedIds, const QMap<kemai::ProfileId, QString> &failures);

    private:
        void onProfilesLoaded(const std::vector<Profile> &profiles);
        void onProfileSaved(ProfileId profileId);
        void onProfileSaveFailed(ProfileId profileId, const QString &message);
        void onProfileRemoved(ProfileId profileId);

        void updateLegacyCandidates();
        void importNextLegacyProfile();

        std::unique_ptr<ProfileModel> m_profileModel;
        std::unique_ptr<ProfileStore> m_profileStore;
        std::vector<Profile>          m_profiles;
        bool                          m_profilesKnown{false};

        // 0.11's file is read once: 0.11 is not expected to change it while this Kemai runs
        std::vector<LegacyProfile>   m_legacyProfiles;
        std::vector<LegacyCandidate> m_legacyCandidates;

        // Import in progress
        bool                     m_importing{false};
        std::vector<Profile>     m_importQueue;
        ProfileId                m_importCurrent;
        QList<ProfileId>         m_importedIds;
        QMap<ProfileId, QString> m_importFailures;
    };
} // namespace kemai
