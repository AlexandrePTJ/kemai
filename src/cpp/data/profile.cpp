// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "profile.h"

namespace kemai
{
    bool Profile::isValid() const
    {
        return !id.isNull() && !name.isEmpty() && !host.isEmpty() && !token.isEmpty();
    }

    QJsonObject Profile::toJson() const
    {
        return {
            {"id",   id.toString()},
            {"name", name         },
            {"host", host         },
        };
    }

    Profile Profile::fromJson(const QJsonObject &json)
    {
        // Mostly for dev. Token will be saved in the keychain store anyway
        auto token = json.contains("token") ? json["token"].toString() : "";

        return {
            .id    = QUuid::fromString(json["id"].toString()),
            .name  = json["name"].toString(),
            .host  = json["host"].toString(),
            .token = token,
        };
    }

} // namespace kemai
