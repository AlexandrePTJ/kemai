#include "kemai/settings.h"

#include <QCoreApplication>
#include <QFile>
#include <QSettings>

using namespace kemai::core;

QSettings getQSettingsInstance(const QString& confPath)
{
    if (confPath.isEmpty() or not QFile::exists(confPath))
    {
        return QSettings(QSettings::IniFormat, QSettings::UserScope, qApp->organizationName(), qApp->applicationName());
    }
    else
    {
        return QSettings(confPath, QSettings::IniFormat);
    }
}

bool Settings::isReady() const
{
    return not host.isEmpty() and not username.isEmpty() and not token.isEmpty();
}

Settings Settings::load(const QString& confPath)
{
    auto qset = getQSettingsInstance(confPath);

    Settings settings;
    settings.host                = qset.value("host").toString();
    settings.username            = qset.value("username").toString();
    settings.token               = qset.value("token").toString();
    settings.closeToSystemTray   = qset.value("closeToSystemTray", false).toBool();
    settings.ignoredVersion      = qset.value("ignoredVersion", "0.0.0").toString();
    settings.trustedCertificates = qset.value("trustedCertificates", {}).toStringList();

    return settings;
}

void Settings::save(const Settings& settings, const QString& confPath)
{
    auto qset = getQSettingsInstance(confPath);
    qset.setValue("host", settings.host);
    qset.setValue("username", settings.username);
    qset.setValue("token", settings.token);
    qset.setValue("closeToSystemTray", settings.closeToSystemTray);
    qset.setValue("ignoredVersion", settings.ignoredVersion);
    qset.setValue("trustedCertificates", settings.trustedCertificates);
}
