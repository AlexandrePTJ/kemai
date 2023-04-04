#pragma once

#include <optional>

#include <QLocale>
#include <QStringList>
#include <QUuid>

namespace kemai {

struct Settings
{
    struct Profile
    {
        QUuid id;
        QString name;
        QString host;
        QString username;
        QString token;
    };
    QList<Profile> profiles;

    QStringList trustedCertificates;

    struct Kemai
    {
        bool closeToSystemTray    = false;
        bool minimizeToSystemTray = false;
        bool checkUpdateAtStartup = true;
        QString ignoredVersion;
        QByteArray geometry;
        QLocale language;
        QUuid lastConnectedProfile;
    } kemai;

    struct Events
    {
        bool stopOnLock                             = false;
        bool stopOnIdle                             = false;
        int idleDelayMinutes                        = 1;
        bool autoRefreshCurrentTimeSheet            = false;
        int autoRefreshCurrentTimeSheetDelaySeconds = 5;
    } events;

    bool isReady() const;

    QList<Profile>::iterator findProfileIt(const QUuid& profileId);
    std::optional<Profile> findProfile(const QUuid& profileId);

    static Settings get();
    static void save(const Settings& settings);
};

} // namespace kemai
