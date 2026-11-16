// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "profileManager.h"

// 3rd party headers
#include <magic_enum/magic_enum.hpp>

namespace kemai
{
    ProfileManager::ProfileManager(QObject *parent):
    ProfileController(parent)
    {
        connect(this, &ProfileController::profileSaveSucceeded, this, &ProfileManager::saveSucceeded);
        connect(this, &ProfileController::profileSaveFailed, this, [this](ProfileId profileId, ProfileSaveError error)
                { emit saveFailed(profileId, reasonKey(error)); });
        connect(this, &ProfileController::legacyImportFinished, this, [this](const QList<ProfileId> &importedIds, const QMap<ProfileId, ProfileSaveError> &failures)
                {
                    QVariantList failureList;
                    for (const auto &[id, reason] : failures.asKeyValueRange())
                    {
                        auto it = std::ranges::find_if(legacyCandidates(), [&id](const LegacyCandidate &candidate)
                                                       { return candidate.profile.id == id; });
                        failureList.append(QVariantMap{
                            {"name",   it != legacyCandidates().end() ? it->finalName : QString()},
                            {"reason", reasonKey(reason)                                        }
                        });
                    }
                    emit legacyImportCompleted(static_cast<int>(importedIds.size()), failureList); });
    }

    QString ProfileManager::reasonKey(ProfileSaveError error)
    {
        auto key = QString::fromUtf8(magic_enum::enum_name(error).data());
        key[0]   = key[0].toLower();
        return key;
    }

    ProfileId ProfileManager::upsert(const ProfileId &profileId, const QString &name, const QString &host, const QString &token)
    {
        return ProfileController::upsert(profileId, name, host, token);
    }

    void ProfileManager::remove(const ProfileId &profileId)
    {
        ProfileController::remove(profileId);
    }

    void ProfileManager::importLegacyProfiles()
    {
        ProfileController::importLegacyProfiles();
    }

    QVariantList ProfileManager::legacyCandidateList() const
    {
        QVariantList list;
        for (const auto &candidate : legacyCandidates())
        {
            auto state = QString::fromUtf8(magic_enum::enum_name(candidate.state));
            state[0]   = state[0].toLower();
            list.append(QVariantMap{
                {"profileId",     candidate.profile.id           },
                {"name",          candidate.profile.name         },
                {"finalName",     candidate.finalName            },
                {"host",          candidate.profile.host         },
                {"state",         state                          },
                {"lastConnected", candidate.profile.lastConnected}
            });
        }
        return list;
    }

} // namespace kemai
