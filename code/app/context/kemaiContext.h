// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// STL headers
#include <memory>

// Qt headers
#include <QObject>
#include <QtQmlIntegration/qqmlintegration.h>

// Project headers
#include <client/kimaiClient.h>
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

    public:
        explicit KemaiContext(QObject *parent = nullptr);
        ~KemaiContext() override;

        Q_INVOKABLE void initialize();
        QString lastAccountName() const;
        bool    justLoggedOut() const;
        QString windowTitle() const;

        // `profileName`: saved profile the credentials come from, empty when typed by hand
        Q_INVOKABLE void login(const QString &host, const QString &token, bool rememberLogin, const QString &profileName = {});
        // Ends the session on purpose: disables auto-login, then emits loggedOut for the UI to leave the session
        Q_INVOKABLE void logout();
        // Releases a session the UI no longer shows (logout or expiry); call once its view is gone
        Q_INVOKABLE void closeSession(kemai::SessionContext *session);

    signals:
        void loginSucceeded(kemai::SessionContext *session);
        void loginFailed(const QString &error);
        void loggedOut();
        void lastSessionChanged();

    private:
        void updateAutoLoginSettings(const QString &host, const QString &token, bool rememberLogin);
        void onSettingsLoaded(const AppSettings &settings);

        std::unique_ptr<AppSettingsStore> m_settingsStore;
        std::unique_ptr<AppSettings>      m_pendingSettings;
        QString                           m_lastAccountName;
        bool                              m_justLoggedOut = false;
    };
} // namespace kemai
