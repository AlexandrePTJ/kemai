// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "appSettingsStore.h"

// 3rd party headers
#include <spdlog/spdlog.h>

// Qt headers
#include <QJsonObject>

// Project helpers
#include <misc/customFmt.h>
#include <misc/jsonHelpers.h>
#include <misc/pathHelpers.h>

namespace
{
    /* Changelog
     * =========
     * 2026-03-21    1.0    Initial version
     */
    const auto gSettingsCurrentVersion     = QString("1.0");
    const auto gSettingsSecretAutoLoginKey = QString("autologin/token");

} // namespace

namespace kemai
{
    AppSettingsStore::AppSettingsStore(QObject *parent):
    QObject(parent),
    m_secretStorage(this)
    {
        connect(&m_secretStorage, &SecretStorage::secretRead, this, &AppSettingsStore::onSecretRead);
        connect(&m_secretStorage, &SecretStorage::readErrorOccurred, this, &AppSettingsStore::onSecretReadError);
    }

    void AppSettingsStore::setStorePath(const QString &storePath)
    {
        m_storePath = storePath;
    }

    void AppSettingsStore::load()
    {
        try
        {
            auto json = JsonHelpers::loadJsonObjectFromFile(m_storePath);

            if (!json.contains("version"))
            {
                throw std::runtime_error("Missing version field");
            }

            auto settingsObject = json["settings"].toObject();
            m_pendingSettings   = AppSettings::fromJson(settingsObject);

            if (m_pendingSettings.autoLogin.has_value())
            {
                m_secretStorage.readSecret(gSettingsSecretAutoLoginKey);
            }
            else
            {
                emit settingsLoaded(m_pendingSettings);
            }
        }
        catch (const std::runtime_error &e)
        {
            spdlog::debug("Could not load app settings from {}: {}", m_storePath, e.what());
        }
    }

    void AppSettingsStore::save(const AppSettings &settings)
    {
        PathHelpers::ensureDirectoryExists(m_storePath);

        QJsonObject json;
        json.insert("version", gSettingsCurrentVersion);
        json.insert("settings", settings.toJson());

        JsonHelpers::saveJsonToFile(json, m_storePath);

        if (settings.autoLogin.has_value())
        {
            m_secretStorage.writeSecret(gSettingsSecretAutoLoginKey, settings.autoLogin->token);
        }
    }

    void AppSettingsStore::onSecretRead(const QString &key, const QString &value)
    {
        if (m_pendingSettings.autoLogin.has_value())
        {
            m_pendingSettings.autoLogin->token = value;
        }
        emit settingsLoaded(m_pendingSettings);
    }

    void AppSettingsStore::onSecretReadError(const QString &key, const QString &message)
    {
        spdlog::debug("Could not read secret {}: {}", key, message);
        emit settingsLoaded(m_pendingSettings);
    }

} // namespace kemai
