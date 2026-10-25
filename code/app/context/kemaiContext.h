// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// STL headers
#include <memory>

// Qt headers
#include <QObject>
#include <QRect>
#include <QVariantList>
#include <QVariantMap>
#include <QWindow>
#include <QtQmlIntegration/qqmlintegration.h>

// Project headers
#include <client/kimaiClient.h>
#include <context/appPreferences.h>
#include <context/sessionContext.h>
#include <storage/appSettingsStore.h>

namespace kemai
{
    class KemaiContext : public QObject
    {
        Q_OBJECT
        QML_ELEMENT

        // Account of the last session, and whether it ended by logout (not persisted): the login view's notice
        Q_PROPERTY(QString lastAccountName READ lastAccountName NOTIFY lastSessionChanged)
        Q_PROPERTY(bool justLoggedOut READ justLoggedOut NOTIFY lastSessionChanged)
        // "Kemai <version>", plus " alpha" for an alpha release
        Q_PROPERTY(QString windowTitle READ windowTitle CONSTANT)
        // Settings dialog: the preferences, and false when the settings file could not be read (changes kept for this session only)
        Q_PROPERTY(kemai::AppPreferences *preferences READ preferences CONSTANT)
        Q_PROPERTY(bool settingsPersistent READ settingsPersistent CONSTANT)
        // Certificate the last login refused ({host, subject, issuer, selfSigned, validFrom, validUntil, sha256, replacesTrusted,
        // previousSha256, rejection: "none" | "expired" | "notyetvalid" | "hostname" | "revoked" | "other"}), empty otherwise
        Q_PROPERTY(QVariantMap refusedCertificate READ refusedCertificate NOTIFY refusedCertificateChanged)
        // Certificates the user trusted: {host, sha256, expiresAt, expired}
        Q_PROPERTY(QVariantList trustedCertificates READ trustedCertificates NOTIFY trustedCertificatesChanged)

    public:
        explicit KemaiContext(QObject *parent = nullptr);
        ~KemaiContext() override;

        Q_INVOKABLE void initialize();
        QString          lastAccountName() const;
        bool             justLoggedOut() const;
        QString          windowTitle() const;
        AppPreferences  *preferences() const;
        bool             settingsPersistent() const;
        QVariantMap      refusedCertificate() const;
        QVariantList     trustedCertificates() const;

        // Trusts the certificate the last login refused (for its host only, never an expired or misnamed one),
        // then logs in again with the same credentials
        Q_INVOKABLE void trustRefusedCertificate();
        // The next connection to `host` asks again
        Q_INVOKABLE void removeTrustedCertificate(const QString &host);

        // `profileName`: saved profile the credentials come from, empty when typed by hand
        Q_INVOKABLE void login(const QString &host, const QString &token, bool rememberLogin, const QString &profileName = {});
        // Ends the session on purpose: disables auto-login, then emits loggedOut for the UI to leave the session
        Q_INVOKABLE void logout();
        // Releases a session the UI no longer shows (logout or expiry); call once its view is gone
        Q_INVOKABLE void closeSession(kemai::SessionContext *session);

        // Applies the saved geometry (fitted to the connected screens; size only where the platform places windows) and shows `window`
        Q_INVOKABLE void showWindowWithSavedGeometry(QWindow *window);
        // `normalGeometry`: geometry of the window when not maximized
        Q_INVOKABLE void saveWindowGeometry(const QRect &normalGeometry, bool maximized);

    signals:
        void loginSucceeded(kemai::SessionContext *session);
        // The login view words it from `kind`; `detail`: Kimai's message, or the https address (HttpsRedirect)
        void loginFailed(kemai::SessionController::NotificationKind kind, const QString &detail);
        void loggedOut();
        void lastSessionChanged();
        void refusedCertificateChanged();
        void trustedCertificatesChanged();

    private:
        void updateAutoLoginSettings(const QString &host, const QString &token, bool rememberLogin);
        void onSettingsLoaded(const AppSettings &settings);

        std::unique_ptr<AppSettingsStore> m_settingsStore;
        std::unique_ptr<AppSettings>      m_pendingSettings;
        std::unique_ptr<AppPreferences>   m_preferences;
        bool                              m_settingsPersistent = true;
        std::optional<CertificateInfo>    m_refusedCertificate;
        // Credentials of the last login, to retry it once a certificate is trusted
        struct LoginRequest
        {
            QString host;
            QString token;
            bool    remember = false;
            QString profileName;
        };
        LoginRequest m_lastLogin;
        QString      m_lastAccountName;
        bool         m_justLoggedOut = false;
    };
} // namespace kemai
