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

    public:
        explicit ProfileManager(QObject *parent = nullptr);

        Q_INVOKABLE void upsert(const ProfileId &profileId, const QString &name, const QString &host, const QString &token);
        Q_INVOKABLE void remove(const ProfileId &profileId);
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
