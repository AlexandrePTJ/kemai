// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "profileManager.h"

namespace kemai
{
    ProfileManager::ProfileManager(QObject *parent):
    ProfileController(parent)
    {
    }

    void ProfileManager::upsert(const ProfileId &profileId, const QString &name, const QString &host, const QString &token)
    {
        ProfileController::upsert(profileId, name, host, token);
    }

    void ProfileManager::remove(const ProfileId &profileId)
    {
        ProfileController::remove(profileId);
    }

} // namespace kemai
