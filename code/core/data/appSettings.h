// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// STL headers
#include <optional>

// Project headers
#include <data/profile.h>

namespace kemai
{
    struct Credentials
    {
        QString host;
        QString token;
    };

    struct AppSettings
    {
        std::optional<Credentials> autoLogin;

        QJsonObject        toJson() const;
        static AppSettings fromJson(const QJsonObject &json);
    };

} // namespace kemai
