// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// Qt headers
#include <QList>
#include <QVersionNumber>

namespace kemai
{

    enum class ApiPlugin
    {
        Unknown,
        TaskManagement
    };

    ApiPlugin pluginByName(const QString &pluginName);

    struct KimaiVersion
    {
        QVersionNumber kimai;

        static KimaiVersion fromJson(const QJsonValue &json);
    };

    struct KimaiUser
    {
        int     id = 0;
        QString username;
        QString language;
        QString timezone;

        static KimaiUser fromJson(const QJsonValue &json);
    };

    struct KimaiPlugin
    {
        QString        name;
        QVersionNumber version;
        ApiPlugin      apiPlugin;

        static KimaiPlugin fromJson(const QJsonValue &json);
    };
    using KimaiPlugins = QList<KimaiPlugin>;

} // namespace kemai
