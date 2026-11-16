// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// STL headers
#include <memory>

// Qt headers
#include <QObject>
#include <QPointer>
#include <QString>
#include <QUrl>
#include <QtQmlIntegration/qqmlintegration.h>

// Project headers
#include <client/kemaiUpdateChecker.h>
#include <context/appPreferences.h>

namespace kemai
{
    /*!
     * State of the update check for the update strip, the account / login menus and the Updates settings: one
     * check at a time, the latest started wins. A check at start is silent unless it finds a version not ignored;
     * a manual one always reports its result, ignored version included. `source` tells where the result shows:
     * the strip (Startup, Menu) or the settings dialog (Settings).
     */
    class UpdateNotifier : public QObject
    {
        Q_OBJECT
        QML_ELEMENT

        // False when built without KEMAI_ENABLE_UPDATE_CHECK: every update entry is hidden
        Q_PROPERTY(bool enabled READ enabled CONSTANT)
        // Development builds never check at start
        Q_PROPERTY(bool devBuild READ devBuild CONSTANT)
        Q_PROPERTY(QString currentVersion READ currentVersion CONSTANT)
        // "alpha", "stable" or "dev"
        Q_PROPERTY(QString channel READ channel CONSTANT)
        Q_PROPERTY(kemai::AppPreferences *preferences READ preferences WRITE setPreferences NOTIFY preferencesChanged)

        Q_PROPERTY(Status status READ status NOTIFY stateChanged)
        Q_PROPERTY(Source source READ source NOTIFY stateChanged)
        // Available: the newer version and its release page
        Q_PROPERTY(QString latestVersion READ latestVersion NOTIFY stateChanged)
        Q_PROPERTY(bool latestPrerelease READ latestPrerelease NOTIFY stateChanged)
        Q_PROPERTY(QUrl releaseUrl READ releaseUrl NOTIFY stateChanged)
        // Failed: why
        Q_PROPERTY(Failure failure READ failure NOTIFY stateChanged)

    public:
        enum Status
        {
            Idle,
            Checking,
            Available,
            UpToDate,
            Failed
        };
        Q_ENUM(Status)

        enum Source
        {
            Startup,
            Menu,
            Settings
        };
        Q_ENUM(Source)

        enum Failure
        {
            NoNetwork,
            ServerError,
            InvalidResponse
        };
        Q_ENUM(Failure)

        explicit UpdateNotifier(QObject *parent = nullptr);
        ~UpdateNotifier() override;

        bool            enabled() const;
        bool            devBuild() const;
        QString         currentVersion() const;
        QString         channel() const;
        AppPreferences *preferences() const;
        void            setPreferences(AppPreferences *preferences);

        Status  status() const;
        Source  source() const;
        QString latestVersion() const;
        bool    latestPrerelease() const;
        QUrl    releaseUrl() const;
        Failure failure() const;

        // Once after start, when allowed: not for development builds, and only with the preference on
        Q_INVOKABLE void checkAtStart();
        Q_INVOKABLE void checkNow(Source source);
        // Back to Idle (dismissed, or the result was seen)
        Q_INVOKABLE void reset();
        // The settings dialog closed: a new version found there moves to the strip (also one still being
        // checked), any other result is dropped
        Q_INVOKABLE void showInStrip();

    signals:
        void preferencesChanged();
        void stateChanged();

    private:
        void run(Source source);
        void setState(Status status);

        std::unique_ptr<KemaiUpdateChecker> m_checker;
        QPointer<AppPreferences>            m_preferences;
        int                                 m_generation = 0;
        Status                              m_status     = Idle;
        Source                              m_source     = Startup;
        KemaiRelease                        m_latest;
        Failure                             m_failure = NoNetwork;
    };
} // namespace kemai
