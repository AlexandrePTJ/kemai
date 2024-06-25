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

    bool hasValidProfile() const;

    QList<Settings::Profile>::iterator findProfileIt(const QUuid& profileId);
    QList<Settings::Profile>::const_iterator findProfileIt(const QUuid& profileId) const;
    std::optional<Settings::Profile> findProfile(const QUuid& profileId) const;
};

class SettingsHandler
{
public:
    static SettingsHandler& instance();

    const Settings& get();
    void set(const Settings& settings);

private:
    std::optional<Settings> mSettings = std::nullopt;
};

} // namespace kemai
