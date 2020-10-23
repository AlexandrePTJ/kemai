#pragma once

#include <QStringList>

namespace kemai::core {

struct Settings
{
    struct Kimai
    {
        QString host;
        QString username;
        QString token;
        QStringList trustedCertificates;
    } kimai;

    struct Kemai
    {
        bool closeToSystemTray;
        QString ignoredVersion;
        QByteArray geometry;
    } kemai;

    bool isReady() const;

    static Settings load();
    static void save(const Settings& settings);
};

} // namespace kemai::core
