// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// STL headers
#include <vector>

// Qt headers
#include <QJsonObject>
#include <QString>

// Project headers
#include <data/profile.h>

namespace kemai
{
    // A profile of Kemai 0.11.x, with only what the import needs: 0.11's legacy API password (username + token) is
    // not even read, so it can neither be used nor leaked
    struct LegacyProfile
    {
        ProfileId id;
        QString   name;
        QString   host;
        QString   apiToken;             // empty when 0.11 only had the legacy API password
        bool      lastConnected{false}; // the profile 0.11 last connected with
    };

    // Reads Kemai 0.11.x's settings.json, never writes it
    class LegacySettingsReader
    {
    public:
        // Parses the file content (versions 1-3); entries without name or host are skipped.
        // Throws std::runtime_error when there is no version >= 1 (not a 0.11 JSON settings file).
        static std::vector<LegacyProfile> profilesFromJson(const QJsonObject &json);

        // Opens the file read-only; empty when it is missing, unreadable or invalid (the reason is logged)
        static std::vector<LegacyProfile> readProfiles(const QString &path);
    };

} // namespace kemai
