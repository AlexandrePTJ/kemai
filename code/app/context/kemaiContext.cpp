// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "kemaiContext.h"

// Project headers
#include <client/kimaiSystem.h>
#include <misc/pathHelpers.h>

namespace kemai
{
    KemaiContext::KemaiContext(QObject *parent):
    QObject(parent),
    m_settingsStore(std::make_unique<AppSettingsStore>(this))
    {
        auto settingsPath = PathHelpers::getFilePath(PathHelpers::FilePathType::AppSettings);
        m_settingsStore->setStorePath(settingsPath);

        connect(m_settingsStore.get(), &AppSettingsStore::settingsLoaded, this, &KemaiContext::onSettingsLoaded);
    }

    KemaiContext::~KemaiContext() = default;

    void KemaiContext::initialize()
    {
        m_settingsStore->load();
    }

    void KemaiContext::login(const QString &host, const QString &token, bool rememberLogin)
    {
        m_pendingClient = std::make_unique<KimaiClient>();
        m_pendingClient->setHost(host);
        m_pendingClient->setToken(token);

        // clang-format off
        m_pendingClient->requestMeUserInfo()
            .then(this, [this, host, token, rememberLogin](KimaiUser user)
                {
                    auto* session = new SessionContext(std::move(m_pendingClient), user, this);
                    emit loginSucceeded(session);
                    updateAutoLoginSettings(host, token, rememberLogin);
                })
            .onFailed(this, [this](const std::exception &e)
                {
                    m_pendingClient.reset();
                    emit loginFailed(QString::fromStdString(e.what()));
                    updateAutoLoginSettings("", "", false);
                });
        // clang-format on
    }

    void KemaiContext::updateAutoLoginSettings(const QString &host, const QString &token, bool rememberLogin)
    {
        if (m_pendingSettings == nullptr)
        {
            m_pendingSettings = std::make_unique<AppSettings>();
        }

        if (rememberLogin)
        {
            m_pendingSettings->autoLogin = Credentials{.host = host, .token = token};
        }
        else
        {
            m_pendingSettings->autoLogin.reset();
        }

        m_settingsStore->save(*m_pendingSettings);
    }

    void KemaiContext::onSettingsLoaded(const AppSettings &settings)
    {
        m_pendingSettings = std::make_unique<AppSettings>(settings);

        if (m_pendingSettings->autoLogin.has_value())
        {
            const auto &credentials = m_pendingSettings->autoLogin.value();
            login(credentials.host, credentials.token, true);
        }
    }

} // namespace kemai
