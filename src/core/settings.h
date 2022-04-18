#pragma once

#include <QLocale>
#include <QStringList>
#include <QUuid>

namespace kemai::core {

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
        bool closeToSystemTray;
        bool minimizeToSystemTray;
        QString ignoredVersion;
        QByteArray geometry;
        QLocale language;
        QUuid lastConnectedProfile;
    } kemai;

    bool isReady() const;
    QList<Profile>::iterator findProfileRef(const QUuid& profileId);

    static Settings load();
    static void save(const Settings& settings);
};

} // namespace kemai::core
