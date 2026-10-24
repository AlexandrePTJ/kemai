// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// STL headers
#include <optional>

// Qt headers
#include <QJsonObject>
#include <QRect>

// Project headers
#include <data/profile.h>

namespace kemai
{
    struct Credentials
    {
        QString host;
        QString token;
    };

    struct WindowGeometry
    {
        QRect normal;            // position and size when not maximized
        bool  maximized = false;
    };

    struct AppSettings
    {
        std::optional<Credentials>    autoLogin;
        std::optional<WindowGeometry> window;
        // Per-computer preferences, read and written by key in the app layer. Kept as a whole so keys written
        // by a newer Kemai (unknown here) survive a save by this one.
        QJsonObject                   preferences;

        QJsonObject        toJson() const;
        static AppSettings fromJson(const QJsonObject &json);
    };

} // namespace kemai
