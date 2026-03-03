// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "kimaiSystem.h"

// Project headers
#include <misc/jsonHelpers.h>

namespace kemai
{

    ApiPlugin pluginByName(const QString &pluginName)
    {
        if (pluginName == "TaskManagementBundle")
        {
            return ApiPlugin::TaskManagement;
        }
        return ApiPlugin::Unknown;
    }

    KimaiVersion KimaiVersion::fromJson(const QJsonValue &json)
    {
        JsonHelpers::checkTypeOrThrow("KimaiVersion", json, {QJsonValue::Object});

        auto jsonObject = json.toObject();
        JsonHelpers::checkKeysOrThrow("KimaiVersion", jsonObject, {"version"});

        KimaiVersion v;
        v.kimai = QVersionNumber::fromString(jsonObject.value("version").toString());
        return v;
    }

    KimaiUser KimaiUser::fromJson(const QJsonValue &json)
    {
        JsonHelpers::checkTypeOrThrow("KimaiUser", json, {QJsonValue::Object, QJsonValue::Double});

        if (json.isDouble())
        {
            KimaiUser user;
            user.id = json.toInt();
            return user;
        }

        auto jsonObject = json.toObject();
        JsonHelpers::checkKeysOrThrow("KimaiUser", jsonObject, {"id", "username", "memberships"});

        KimaiUser user;
        user.id       = jsonObject.value("id").toInt();
        user.username = jsonObject.value("username").toString();
        user.language = jsonObject.value("language").toString();
        user.timezone = jsonObject.value("timezone").toString();
        return user;
    }

    KimaiPlugin KimaiPlugin::fromJson(const QJsonValue &json)
    {
        JsonHelpers::checkTypeOrThrow("KimaiPlugin", json, {QJsonValue::Object});

        auto jsonObject = json.toObject();
        JsonHelpers::checkKeysOrThrow("KimaiPlugin", jsonObject, {"name", "version"});

        KimaiPlugin plugin;
        plugin.name      = jsonObject.value("name").toString();
        plugin.version   = QVersionNumber::fromString(jsonObject.value("version").toString());
        plugin.apiPlugin = pluginByName(plugin.name);
        return plugin;
    }

} // namespace kemai
