// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "profile.h"

// magic_enum headers
#include <magic_enum/magic_enum.hpp>
#include <utility>

namespace
{

    QVariantMap getProfileAsVariantMap(const kemai::Profile &profile)
    {
        return {
            {"id",    profile.id   },
            {"name",  profile.name },
            {"host",  profile.host },
            {"token", profile.token}
        };
    }

} // namespace

namespace kemai
{
    ProfileModel::ProfileModel(QObject *parent):
    QAbstractListModel(parent)
    {
    }

    ProfileModel::~ProfileModel() = default;

    void ProfileModel::setProfiles(const std::vector<Profile> &profiles)
    {
        beginResetModel();
        m_profiles = profiles;
        endResetModel();
    }

    QVariantMap ProfileModel::getByIndex(int index) const
    {
        if (index < 0 || index >= m_profiles.size())
        {
            return {};
        }
        return getProfileAsVariantMap(m_profiles[index]);
    }

    QVariantMap ProfileModel::getById(const ProfileId &id) const
    {
        for (const auto &profile : m_profiles)
        {
            if (profile.id == id)
            {
                return getProfileAsVariantMap(profile);
            }
        }
        return {};
    }

    int ProfileModel::rowCount(const QModelIndex &parent) const
    {
        return static_cast<int>(m_profiles.size());
    }

    QVariant ProfileModel::data(const QModelIndex &index, int role) const
    {
        if (!index.isValid() || index.row() > m_profiles.size())
        {
            return {};
        }

        const auto &profile = m_profiles[index.row()];
        switch (static_cast<ProfileRole>(role))
        {
        case ProfileRole::ProfileId:
            return profile.id;
        case ProfileRole::Name:
            return profile.name;
        case ProfileRole::Host:
            return profile.host;
        case ProfileRole::Token:
            return profile.token;
        default:
            return {};
        }
    }

    QHash<int, QByteArray> ProfileModel::roleNames() const
    {
        return {
            {magic_enum::enum_integer(ProfileRole::ProfileId), "profileId"},
            {magic_enum::enum_integer(ProfileRole::Name),      "name"     },
            {magic_enum::enum_integer(ProfileRole::Host),      "host"     },
            {magic_enum::enum_integer(ProfileRole::Token),     "token"    }
        };
    }

} // namespace kemai
