#pragma once

#include <QStringList>

namespace kemai::core {

struct Settings
{
    QString host;
    QString username;
    QString token;
    bool closeToSystemTray;
    QStringList trustedCertificates;

    QString ignoredVersion;

    bool isReady() const;

    static Settings load(const QString& confPath = "");
    static void save(const Settings& settings, const QString& confPath = "");
};

} // namespace kemai::core
