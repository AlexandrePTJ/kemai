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
        std::optional<int> autoRefreshDelaySeconds = std::nullopt;
    } kemai;

    struct Events
    {
        bool stopOnLock      = false;
        bool stopOnIdle      = false;
        int idleDelayMinutes = 1;
    } events;

    bool isReady() const;
    std::optional<Profile> findProfile(const QUuid& profileId);

    static Settings load();
    static void save(const Settings& settings);
};

} // namespace kemai
