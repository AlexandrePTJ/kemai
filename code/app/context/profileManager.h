// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// Qt headers
#include <QtQmlIntegration/qqmlintegration.h>

// Project headers
#include <context/profileController.h>

namespace kemai
{
    class ProfileManager : public ProfileController
    {
        Q_OBJECT
        QML_ELEMENT

        Q_PROPERTY(kemai::ProfileModel *model READ profileModel CONSTANT)
        // Kemai 0.11's profiles: [{profileId, name, finalName, host, state ("ready" | "noToken" | "alreadyImported"),
        // lastConnected}]
        Q_PROPERTY(QVariantList legacyCandidates READ legacyCandidateList NOTIFY legacyCandidatesChanged)
        Q_PROPERTY(bool hasLegacyCandidates READ hasLegacyCandidates NOTIFY legacyCandidatesChanged)
        // The existing profiles are loaded: the candidates' states are final
        Q_PROPERTY(bool legacyCandidatesKnown READ legacyCandidatesKnown NOTIFY legacyCandidatesChanged)
        Q_PROPERTY(QString legacyLastProfileName READ legacyLastProfileName NOTIFY legacyCandidatesChanged)

    public:
        explicit ProfileManager(QObject *parent = nullptr);

        // Returns the id of the saved profile (new for a new one), which saveSucceeded / saveFailed carry
        Q_INVOKABLE kemai::ProfileId upsert(const ProfileId &profileId, const QString &name, const QString &host, const QString &token);
        Q_INVOKABLE void             remove(const ProfileId &profileId);
        Q_INVOKABLE void             importLegacyProfiles();

        QVariantList legacyCandidateList() const;

        // "profilesFileUnreadable" | "profilesFileUnwritable" | "keychainRefused", worded by the UI
        static QString reasonKey(ProfileSaveError error);

    signals:
        // failures: [{name, reason}] (reason: a reasonKey)
        void legacyImportCompleted(int importedCount, const QVariantList &failures);
        // Outcome of upsert(); `reason`: a reasonKey
        void saveSucceeded(kemai::ProfileId profileId);
        void saveFailed(kemai::ProfileId profileId, const QString &reason);
    };

    // QML registration for the core model used as a QML property type
    struct ProfileModelForeign
    {
        Q_GADGET
        QML_FOREIGN(ProfileModel)
        QML_NAMED_ELEMENT(ProfileModel)
        QML_UNCREATABLE("ProfileModel is exposed through ProfileManager")
    };
} // namespace kemai
