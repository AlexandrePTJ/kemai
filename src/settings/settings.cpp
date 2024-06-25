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

using namespace kemai;

/*
 * Config file version
 */
static const auto gCfgVersion_0      = 0; // Ini format
static const auto gCfgVersion_1      = 1; // Migrate to JSON
static const auto gCfgVersion_2      = 2; // Add AutoRefreshCurrentTimeSheet setting
static const auto gCfgVersion_3      = 3; // Add API Token to profile
static const auto gCurrentCfgVersion = gCfgVersion_3;

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

Settings loadFromLegacyIni(const QSettings& qset)
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

    return settings;
}

/*
 * Class impl
 */

bool Settings::hasValidProfile() const
{
    if (!profiles.empty())
    {
        const auto profile        = profiles.begin();
        const auto haveHost       = !profile->host.isEmpty();
        const auto haveLegacyAuth = !profile->username.isEmpty() && !profile->token.isEmpty();
        const auto haveAPIToken   = !profile->apiToken.isEmpty();

        return haveHost && (haveLegacyAuth || haveAPIToken);
    }
    return false;
}

QList<Settings::Profile>::iterator Settings::findProfileIt(const QUuid& profileId)
{
    return std::find_if(profiles.begin(), profiles.end(), [&profileId](const auto& profile) { return profile.id == profileId; });
}

QList<Settings::Profile>::const_iterator Settings::findProfileIt(const QUuid& profileId) const
{
    return std::find_if(profiles.begin(), profiles.end(), [&profileId](const auto& profile) { return profile.id == profileId; });
}

std::optional<Settings::Profile> Settings::findProfile(const QUuid& profileId) const
{
    auto it = findProfileIt(profileId);
    if (it != profiles.end())
    {
        return *it;
    }
    return std::nullopt;
}

SettingsHandler& SettingsHandler::instance()
{
    static SettingsHandler gSettingsHandler;
    return gSettingsHandler;
}

const Settings& SettingsHandler::get()
{
    if (!mSettings.has_value())
    {
        mSettings = Settings{};

        auto qset             = getQSettingsInstance();
        auto jsonSettingsPath = getJsonSettingsPath();

        // Migrate from ini settings to json
        if (QFile::exists(qset.fileName()) && !QFile::exists(jsonSettingsPath))
        {
            auto settings = loadFromLegacyIni(qset);
            set(settings);
            return mSettings.value();
        }

        // Load from previous save
        if (QFile::exists(jsonSettingsPath))
        {
            QFile jsonFile(jsonSettingsPath);
            jsonFile.open(QIODevice::ReadOnly | QIODevice::Text);
            auto jsonDocument = QJsonDocument::fromJson(jsonFile.readAll());
            auto root         = jsonDocument.object();
            jsonFile.close();

            const auto cfgVersion = root.value("version").toInt();

            for (const auto& certificationValue : root.value("trustedCertificates").toArray())
            {
                mSettings->trustedCertificates.append(certificationValue.toString());
            }

            auto kemaiObject                      = root.value("kemai").toObject();
            mSettings->kemai.closeToSystemTray    = kemaiObject.value("closeToSystemTray").toBool();
            mSettings->kemai.minimizeToSystemTray = kemaiObject.value("minimizeToSystemTray").toBool();
            mSettings->kemai.checkUpdateAtStartup = kemaiObject.value("checkUpdateAtStartup").toBool();
            mSettings->kemai.ignoredVersion       = kemaiObject.value("ignoredVersion").toString();
            mSettings->kemai.geometry             = QByteArray::fromBase64(kemaiObject.value("geometry").toString().toLocal8Bit());
            mSettings->kemai.language             = QLocale(kemaiObject.value("language").toString());
            if (kemaiObject.contains("lastConnectedProfile"))
            {
                mSettings->kemai.lastConnectedProfile = QUuid(kemaiObject.value("lastConnectedProfile").toString());
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

                if (cfgVersion >= gCfgVersion_3)
                {
                    profile.apiToken = profileObject.value("apiToken").toString();
                }

                mSettings->profiles << profile;
            }

            if (root.contains("events"))
            {
                auto eventsObject                  = root.value("events").toObject();
                mSettings->events.stopOnLock       = eventsObject.value("stopOnLock").toBool();
                mSettings->events.stopOnIdle       = eventsObject.value("stopOnIdle").toBool();
                mSettings->events.idleDelayMinutes = eventsObject.value("idleDelayMinutes").toInt();

                if (cfgVersion >= gCfgVersion_2)
                {
                    mSettings->events.autoRefreshCurrentTimeSheet             = eventsObject.value("autoRefreshCurrentTimeSheet").toBool();
                    mSettings->events.autoRefreshCurrentTimeSheetDelaySeconds = eventsObject.value("autoRefreshCurrentTimeSheetDelaySeconds").toInt();
                }
            }
        }
    }
    return mSettings.value();
}

void SettingsHandler::set(const Settings& settings)
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
        profileObject["apiToken"] = profile.apiToken;
        profilesArray.append(profileObject);
    }

    QJsonObject kemaiObject;
    kemaiObject["closeToSystemTray"]    = settings.kemai.closeToSystemTray;
    kemaiObject["minimizeToSystemTray"] = settings.kemai.minimizeToSystemTray;
    kemaiObject["checkUpdateAtStartup"] = settings.kemai.checkUpdateAtStartup;
    kemaiObject["ignoredVersion"]       = settings.kemai.ignoredVersion;
    kemaiObject["geometry"]             = QString(settings.kemai.geometry.toBase64());
    kemaiObject["language"]             = settings.kemai.language.name();
    kemaiObject["lastConnectedProfile"] = settings.kemai.lastConnectedProfile.toString();

    QJsonObject eventsObject;
    eventsObject["stopOnLock"]                              = settings.events.stopOnLock;
    eventsObject["stopOnIdle"]                              = settings.events.stopOnIdle;
    eventsObject["idleDelayMinutes"]                        = settings.events.idleDelayMinutes;
    eventsObject["autoRefreshCurrentTimeSheet"]             = settings.events.autoRefreshCurrentTimeSheet;
    eventsObject["autoRefreshCurrentTimeSheetDelaySeconds"] = settings.events.autoRefreshCurrentTimeSheetDelaySeconds;

    QJsonObject root;
    root["version"]             = gCurrentCfgVersion;
    root["profiles"]            = profilesArray;
    root["trustedCertificates"] = QJsonArray::fromStringList(settings.trustedCertificates);
    root["kemai"]               = kemaiObject;
    root["events"]              = eventsObject;

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
    testStream << QJsonDocument(root).toJson();

    jsonFile.close();

    mSettings = settings;
}
