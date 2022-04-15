#pragma once

#include <QLocale>
#include <QStringList>

namespace kemai::core {

struct Settings
{
    struct Profile
    {
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
    } kemai;

    bool isReady() const;

    static Settings load();
    static void save(const Settings& settings);
};

} // namespace kemai::core
