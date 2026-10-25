// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "appSettingsStore.h"

// 3rd party headers
#include <spdlog/spdlog.h>

// Qt headers
#include <QFileInfo>
#include <QJsonObject>
#include <QVersionNumber>

// Project helpers
#include <misc/customFmt.h>
#include <misc/jsonHelpers.h>
#include <misc/pathHelpers.h>

namespace
{
    /* Changelog
     * =========
     * 2026-03-21    1.0    Initial version
     * 2026-10-24    1.0    File renamed from settings.json (Kemai 0.11.x's file) to appSettings.json
     * 2026-10-24    1.1    Add `window`: main window geometry (x, y, width, height of the normal state, maximized)
     * 2026-10-24    1.2    Add `preferences`: per-computer preferences by key (unknown keys are kept on save)
     * 2026-10-24    1.3    Add `trustedCertificates`: [{host, sha256, pem, expiresAt}], certificates trusted per host
     *
     * Versioning policy: same as the profiles file (see profileStore.cpp).
     */
    const auto gSettingsCurrentVersion     = QVersionNumber(1, 3);
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
            m_pendingSettings = settingsFromJson(JsonHelpers::loadJsonObjectFromFile(m_storePath));

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

    AppSettings AppSettingsStore::readFile() const
    {
        try
        {
            return settingsFromJson(JsonHelpers::loadJsonObjectFromFile(m_storePath));
        }
        catch (const std::runtime_error &e)
        {
            spdlog::debug("Could not read app settings from {}: {}", m_storePath, e.what());
            return {};
        }
    }

    AppSettings AppSettingsStore::settingsFromJson(const QJsonObject &json)
    {
        const auto versionString = json.value("version").toString();
        const auto version       = QVersionNumber::fromString(versionString);
        if (version.isNull())
        {
            throw std::runtime_error(fmt::format("Missing or invalid settings version '{}'", versionString));
        }

        if (version.majorVersion() > gSettingsCurrentVersion.majorVersion())
        {
            throw std::runtime_error(fmt::format("Settings version {} was written by a newer Kemai (supports up to {}.x)", versionString, gSettingsCurrentVersion.majorVersion()));
        }

        return AppSettings::fromJson(json.value("settings").toObject());
    }

    bool AppSettingsStore::canSave() const
    {
        if (!QFileInfo::exists(m_storePath))
        {
            return true;
        }
        try
        {
            settingsFromJson(JsonHelpers::loadJsonObjectFromFile(m_storePath));
            return true;
        }
        catch (const std::runtime_error &)
        {
            return false;
        }
    }

    void AppSettingsStore::save(const AppSettings &settings)
    {
        if (QFileInfo::exists(m_storePath))
        {
            try
            {
                settingsFromJson(JsonHelpers::loadJsonObjectFromFile(m_storePath));
            }
            catch (const std::runtime_error &e)
            {
                // Corrupt or newer-format file: keep it, the settings only live for this session.
                spdlog::error("Settings not saved, {} left untouched: {}", m_storePath, e.what());
                return;
            }
        }
        else
        {
            PathHelpers::ensureDirectoryExists(m_storePath);
        }

        QJsonObject json;
        json.insert("version", gSettingsCurrentVersion.toString());
        json.insert("settings", settings.toJson());

        JsonHelpers::saveJsonToFile(json, m_storePath);

        if (settings.autoLogin.has_value() && !settings.autoLogin->token.isEmpty())
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
