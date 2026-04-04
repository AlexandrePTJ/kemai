// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// STL headers
#include <memory>

// Qt headers
#include <QObject>
#include <QtQmlIntegration/qqmlintegration.h>

// Project headers
#include "client/kimaiClient.h"
#include "sessionContext.h"
#include <storage/appSettingsStore.h>

namespace kemai
{
    class KemaiContext : public QObject
    {
        Q_OBJECT
        QML_ELEMENT

    public:
        explicit KemaiContext(QObject *parent = nullptr);
        ~KemaiContext() override;

        Q_INVOKABLE void initialize();
        Q_INVOKABLE void login(const QString &host, const QString &token, bool rememberLogin);

    signals:
        void loginSucceeded(SessionContext *session);
        void loginFailed(const QString &error);

    private:
        void updateAutoLoginSettings(const QString &host, const QString &token, bool rememberLogin);
        void onSettingsLoaded(const AppSettings &settings);

        std::unique_ptr<AppSettingsStore> m_settingsStore;
        std::unique_ptr<AppSettings>      m_pendingSettings;
    };
} // namespace kemai
