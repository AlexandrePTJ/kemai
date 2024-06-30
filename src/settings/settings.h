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
        QString apiToken; // Since Kimai 2.14
    };
    std::vector<Profile> profiles;

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
};

class SettingsHelper
{
public:
    static Settings load();
    static void save(const Settings& settings);
};

} // namespace kemai
