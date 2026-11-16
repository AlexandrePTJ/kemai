// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "legacySettingsReader.h"

// 3rd party headers
#include <spdlog/spdlog.h>

// Qt headers
#include <QFileInfo>
#include <QJsonArray>

// Project helpers
#include <misc/customFmt.h>
#include <misc/jsonHelpers.h>

namespace
{
    // Version 0 was an INI file, migrated to JSON by 0.11 itself; 3 added the API token
    constexpr auto gFirstJsonVersion     = 1;
    constexpr auto gFirstApiTokenVersion = 3;
} // namespace

namespace kemai
{
    std::vector<LegacyProfile> LegacySettingsReader::profilesFromJson(const QJsonObject &json)
    {
        const auto version = json.value("version").toInt();
        if (version < gFirstJsonVersion)
        {
            throw std::runtime_error("Missing or invalid settings version");
        }

        const auto lastConnected = QUuid(json.value("kemai").toObject().value("lastConnectedProfile").toString());

        std::vector<LegacyProfile> profiles;
        for (const auto &profileValue : json.value("profiles").toArray())
        {
            const auto profileObject = profileValue.toObject();

            LegacyProfile profile;
            profile.id   = QUuid(profileObject.value("id").toString());
            profile.name = profileObject.value("name").toString();
            profile.host = profileObject.value("host").toString();
            if (profile.name.isEmpty() || profile.host.isEmpty())
            {
                continue;
            }
            if (version >= gFirstApiTokenVersion)
            {
                profile.apiToken = profileObject.value("apiToken").toString();
            }
            profile.lastConnected = !profile.id.isNull() && profile.id == lastConnected;
            profiles.push_back(profile);
        }
        return profiles;
    }

    std::vector<LegacyProfile> LegacySettingsReader::readProfiles(const QString &path)
    {
        if (!QFileInfo::exists(path))
        {
            spdlog::debug("No Kemai 0.11 settings at {}", path);
            return {};
        }

        try
        {
            return profilesFromJson(JsonHelpers::loadJsonObjectFromFile(path));
        }
        catch (const std::runtime_error &e)
        {
            spdlog::warn("Kemai 0.11 settings {} not read: {}", path, e.what());
            return {};
        }
    }

} // namespace kemai
