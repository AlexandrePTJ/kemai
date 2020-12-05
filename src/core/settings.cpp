#include "settings.h"

#include <QCoreApplication>
#include <QFile>
#include <QSettings>

using namespace kemai::core;

QSettings getQSettingsInstance()
{
    return QSettings(QSettings::IniFormat, QSettings::UserScope, qApp->organizationName(), qApp->applicationName());
}

bool Settings::isReady() const
{
    return not kimai.host.isEmpty() and not kimai.username.isEmpty() and not kimai.token.isEmpty();
}

Settings Settings::load()
{
    auto qset = getQSettingsInstance();

    Settings settings;

    QString kimaiGrpPrefix, kemaiGrpPrefix;

    if (qset.childGroups().contains("kimai"))
        kimaiGrpPrefix = "kimai/";
    if (qset.childGroups().contains("kemai"))
        kemaiGrpPrefix = "kemai/";

    settings.kimai.host                = qset.value(kimaiGrpPrefix + "host").toString();
    settings.kimai.username            = qset.value(kimaiGrpPrefix + "username").toString();
    settings.kimai.token               = qset.value(kimaiGrpPrefix + "token").toString();
    settings.kimai.trustedCertificates = qset.value(kimaiGrpPrefix + "trustedCertificates", {}).toStringList();

    settings.kemai.closeToSystemTray = qset.value(kemaiGrpPrefix + "closeToSystemTray", false).toBool();
    settings.kemai.ignoredVersion    = qset.value(kemaiGrpPrefix + "ignoredVersion", "0.0.0").toString();
    settings.kemai.geometry          = qset.value(kemaiGrpPrefix + "geometry").toByteArray();

    return settings;
}

void Settings::save(const Settings& settings)
{
    auto qset = getQSettingsInstance();
    qset.setValue("kimai/host", settings.kimai.host);
    qset.setValue("kimai/username", settings.kimai.username);
    qset.setValue("kimai/token", settings.kimai.token);
    qset.setValue("kimai/trustedCertificates", settings.kimai.trustedCertificates);
    qset.setValue("kemai/closeToSystemTray", settings.kemai.closeToSystemTray);
    qset.setValue("kemai/ignoredVersion", settings.kemai.ignoredVersion);
    qset.setValue("kemai/geometry", settings.kemai.geometry);
}
