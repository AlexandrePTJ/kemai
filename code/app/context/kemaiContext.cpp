// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "kemaiContext.h"

// spdlog headers
#include <spdlog/spdlog.h>

// Qt headers
#include <QGuiApplication>
#include <QScreen>
#include <QUrl>

// Project headers
#include "kemaiConfig.h"
#include <client/kimaiSystem.h>
#include <misc/customFmt.h>
#include <misc/pathHelpers.h>
#include <misc/windowPlacement.h>

namespace
{
    // Wayland compositors place windows themselves: an application cannot set nor read its window position
    bool platformPlacesWindows()
    {
        return QGuiApplication::platformName().startsWith("wayland");
    }
} // namespace

namespace kemai
{
    KemaiContext::KemaiContext(QObject *parent):
    QObject(parent),
    m_settingsStore(std::make_unique<AppSettingsStore>(this)),
    m_preferences(std::make_unique<AppPreferences>(this))
    {
        auto settingsPath = PathHelpers::getFilePath(PathHelpers::FilePathType::AppSettings);
        m_settingsStore->setStorePath(settingsPath);

        connect(m_settingsStore.get(), &AppSettingsStore::settingsLoaded, this, &KemaiContext::onSettingsLoaded);

        // Read now: the window geometry is needed before the window shows, the keychain read comes later
        m_pendingSettings    = std::make_unique<AppSettings>(m_settingsStore->readFile());
        m_settingsPersistent = m_settingsStore->canSave();
        m_preferences->setJson(m_pendingSettings->preferences);

        connect(m_preferences.get(), &AppPreferences::changed, this,
                [this]
                {
                    m_pendingSettings->preferences = m_preferences->json();
                    m_settingsStore->save(*m_pendingSettings);
                });
    }

    KemaiContext::~KemaiContext() = default;

    void KemaiContext::initialize()
    {
        m_settingsStore->load();
    }

    AppPreferences *KemaiContext::preferences() const
    {
        return m_preferences.get();
    }

    bool KemaiContext::settingsPersistent() const
    {
        return m_settingsPersistent;
    }

    QString KemaiContext::lastAccountName() const
    {
        return m_lastAccountName;
    }

    bool KemaiContext::justLoggedOut() const
    {
        return m_justLoggedOut;
    }

    void KemaiContext::login(const QString &host, const QString &token, bool rememberLogin, const QString &profileName)
    {
        auto client = std::make_shared<KimaiClient>();
        client->setHost(host);
        client->setToken(token);

        // clang-format off
        client->requestMeUserInfo()
            .then(this, [this, client, host, token, rememberLogin, profileName](KimaiUser user)
                {
                    spdlog::info("Logged in to {} as {}", QUrl(host).toString(QUrl::RemoveUserInfo), user.username);
                    auto* session = new SessionContext(std::move(client), user, this);
                    session->setAccount(profileName, host);
                    m_lastAccountName = session->accountName();
                    m_justLoggedOut   = false;
                    emit lastSessionChanged();
                    emit loginSucceeded(session);
                    updateAutoLoginSettings(host, token, rememberLogin);
                })
            .onFailed(this, [this, host](const KimaiApiError &e)
                {
                    spdlog::error("Login to {} failed: {}", QUrl(host).toString(QUrl::RemoveUserInfo), e.what());
                    emit loginFailed(SessionController::notificationKindOf(e.kind()), e.serverMessage());
                    updateAutoLoginSettings("", "", false);
                })
            .onFailed(this, [this, host](const std::exception &e)
                {
                    spdlog::error("Login to {} failed: {}", QUrl(host).toString(QUrl::RemoveUserInfo), e.what());
                    emit loginFailed(SessionController::NotificationKind::UnknownError, {});
                    updateAutoLoginSettings("", "", false);
                });
        // clang-format on
    }

    void KemaiContext::logout()
    {
        spdlog::info("Logging out");
        updateAutoLoginSettings("", "", false);
        m_justLoggedOut = true;
        emit lastSessionChanged();
        emit loggedOut();
    }

    void KemaiContext::closeSession(SessionContext *session)
    {
        if (session != nullptr)
        {
            session->deleteLater();
        }
    }

    void KemaiContext::showWindowWithSavedGeometry(QWindow *window)
    {
        if (window == nullptr)
        {
            return;
        }

        if (const auto &saved = m_pendingSettings->window; saved.has_value())
        {
            QList<QRect> screens;
            if (const auto *primary = QGuiApplication::primaryScreen(); primary != nullptr)
            {
                screens.append(primary->availableGeometry());
            }
            for (const auto *screen : QGuiApplication::screens())
            {
                if (screen != QGuiApplication::primaryScreen())
                {
                    screens.append(screen->availableGeometry());
                }
            }

            const auto placement = WindowPlacement::fit(saved->normal, screens);
            if (placement.positionKept && !platformPlacesWindows())
            {
                window->setGeometry(placement.rect);
            }
            else
            {
                window->resize(placement.rect.size());
            }

            if (saved->maximized)
            {
                window->showMaximized();
                return;
            }
        }
        window->show();
    }

    void KemaiContext::saveWindowGeometry(const QRect &normalGeometry, bool maximized)
    {
        if (!normalGeometry.isValid())
        {
            return;
        }

        auto normal = normalGeometry;
        // The position read on Wayland is meaningless: keep the one saved by an X11, Windows or macOS session
        if (platformPlacesWindows())
        {
            normal.moveTopLeft(m_pendingSettings->window ? m_pendingSettings->window->normal.topLeft() : QPoint());
        }

        m_pendingSettings->window = WindowGeometry{.normal = normal, .maximized = maximized};
        m_settingsStore->save(*m_pendingSettings);
    }

    QString KemaiContext::windowTitle() const
    {
        const auto channel = QStringLiteral(KEMAI_CHANNEL);
        auto       title   = QString("Kemai %1").arg(KEMAI_VERSION);
        // A development version already reads dev-<commit>
        if (channel == "alpha")
        {
            title += " " + channel;
        }
        return title;
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
        // Same file as read at construction, now with the token; preferences changed meanwhile are kept
        m_pendingSettings              = std::make_unique<AppSettings>(settings);
        m_pendingSettings->preferences = m_preferences->json();

        if (m_pendingSettings->autoLogin.has_value())
        {
            const auto &credentials = m_pendingSettings->autoLogin.value();
            login(credentials.host, credentials.token, true);
        }
    }

} // namespace kemai
