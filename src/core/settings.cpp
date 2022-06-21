#include "settings.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QStandardPaths>

using namespace kemai::core;

/*
 * Static helpers
 */
QSettings getQSettingsInstance()
{
    return {QSettings::IniFormat, QSettings::UserScope, qApp->organizationName(), qApp->applicationName()};
}

QString getJsonSettingsPath()
{
    return QDir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)).absoluteFilePath("settings.json");
}

void migrateIniToJson(const QSettings& qset)
{
    Settings settings;

    QString kimaiGrpPrefix;
    QString kemaiGrpPrefix;

    if (qset.childGroups().contains("kimai"))
    {
        kimaiGrpPrefix = "kimai/";
    }
    if (qset.childGroups().contains("kemai"))
    {
        kemaiGrpPrefix = "kemai/";
    }

    Settings::Profile defaultProfile;
    defaultProfile.id       = QUuid::createUuid();
    defaultProfile.name     = "default";
    defaultProfile.host     = qset.value(kimaiGrpPrefix + "host").toString();
    defaultProfile.username = qset.value(kimaiGrpPrefix + "username").toString();
    defaultProfile.token    = qset.value(kimaiGrpPrefix + "token").toString();
    settings.profiles.push_back(defaultProfile);

    settings.trustedCertificates = qset.value(kimaiGrpPrefix + "trustedCertificates", {}).toStringList();

    settings.kemai.closeToSystemTray    = qset.value(kemaiGrpPrefix + "closeToSystemTray", false).toBool();
    settings.kemai.minimizeToSystemTray = qset.value(kemaiGrpPrefix + "minimizeToSystemTray", false).toBool();
    settings.kemai.ignoredVersion       = qset.value(kemaiGrpPrefix + "ignoredVersion", "0.0.0").toString();
    settings.kemai.geometry             = qset.value(kemaiGrpPrefix + "geometry").toByteArray();
    settings.kemai.language             = qset.value(kemaiGrpPrefix + "language", QLocale::system()).toLocale();

    // Save to json format
    Settings::save(settings);
}

/*
 * Class impl
 */

bool Settings::isReady() const
{
    if (!profiles.isEmpty())
    {
        auto profile = profiles.first();
        return !profile.host.isEmpty() && !profile.username.isEmpty() && !profile.token.isEmpty();
    }
    return false;
}

QList<Settings::Profile>::iterator Settings::findProfileRef(const QUuid& profileId)
{
    return std::find_if(profiles.begin(), profiles.end(), [&profileId](const Settings::Profile& profile) { return profile.id == profileId; });
}

Settings Settings::load()
{
    auto qset             = getQSettingsInstance();
    auto jsonSettingsPath = getJsonSettingsPath();

    // Migrate from ini settings to json
    if (QFile::exists(qset.fileName()) && !QFile::exists(jsonSettingsPath))
    {
        migrateIniToJson(qset);
    }

    // First run
    if (!QFile::exists(jsonSettingsPath))
    {
        return {};
    }

    QFile jsonFile(jsonSettingsPath);
    jsonFile.open(QIODevice::ReadOnly | QIODevice::Text);
    auto jsonDocument = QJsonDocument::fromJson(jsonFile.readAll());
    auto root         = jsonDocument.object();
    jsonFile.close();

    Settings settings;

    for (const auto& certificationValue : root.value("trustedCertificates").toArray())
    {
        settings.trustedCertificates.append(certificationValue.toString());
    }

    auto kemaiObject                    = root.value("kemai").toObject();
    settings.kemai.closeToSystemTray    = kemaiObject.value("closeToSystemTray").toBool();
    settings.kemai.minimizeToSystemTray = kemaiObject.value("minimizeToSystemTray").toBool();
    settings.kemai.ignoredVersion       = kemaiObject.value("ignoredVersion").toString();
    settings.kemai.geometry             = QByteArray::fromBase64(kemaiObject.value("geometry").toString().toLocal8Bit());
    settings.kemai.language             = QLocale(kemaiObject.value("language").toString());
    if (kemaiObject.contains("lastConnectedProfile"))
    {
        settings.kemai.lastConnectedProfile = QUuid(kemaiObject.value("lastConnectedProfile").toString());
    }

    for (const auto& profileValue : root.value("profiles").toArray())
    {
        const auto profileObject = profileValue.toObject();

        Settings::Profile profile;
        profile.id       = QUuid(profileObject.value("id").toString());
        profile.name     = profileObject.value("name").toString();
        profile.host     = profileObject.value("host").toString();
        profile.username = profileObject.value("username").toString();
        profile.token    = profileObject.value("token").toString();
        settings.profiles << profile;
    }

    return settings;
}

void Settings::save(const Settings& settings)
{
    QJsonArray profilesArray;
    for (const auto& profile : settings.profiles)
    {
        QJsonObject profileObject;
        profileObject["id"]       = profile.id.toString();
        profileObject["name"]     = profile.name;
        profileObject["host"]     = profile.host;
        profileObject["username"] = profile.username;
        profileObject["token"]    = profile.token;
        profilesArray.append(profileObject);
    }

    QJsonObject kemaiObject;
    kemaiObject["closeToSystemTray"]    = settings.kemai.closeToSystemTray;
    kemaiObject["minimizeToSystemTray"] = settings.kemai.minimizeToSystemTray;
    kemaiObject["ignoredVersion"]       = settings.kemai.ignoredVersion;
    kemaiObject["geometry"]             = QString(settings.kemai.geometry.toBase64());
    kemaiObject["language"]             = settings.kemai.language.name();
    kemaiObject["lastConnectedProfile"] = settings.kemai.lastConnectedProfile.toString();

    QJsonObject root;
    root["version"]             = 1;
    root["profiles"]            = profilesArray;
    root["trustedCertificates"] = QJsonArray::fromStringList(settings.trustedCertificates);
    root["kemai"]               = kemaiObject;

    QJsonDocument jsonDocument(root);

    QFileInfo jsonFileInfo(getJsonSettingsPath());
    if (!jsonFileInfo.exists())
    {
        if (!jsonFileInfo.absoluteDir().exists())
        {
            jsonFileInfo.absoluteDir().mkpath(".");
        }
    }

    QFile jsonFile(jsonFileInfo.filePath());
    jsonFile.open(QIODevice::WriteOnly | QIODevice::Text);

    QTextStream testStream(&jsonFile);
    testStream << jsonDocument.toJson();

    jsonFile.close();
}
