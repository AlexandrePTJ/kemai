#include "settingsviewbridge.h"

#include "client/kimairequestfactory.h"

using namespace kemai::qmlapp;
using namespace kemai::client;
using namespace kemai::core;

SettingsViewBridge::SettingsViewBridge()
{
    mSettings = Settings::load();

    connect(&mClient, &KimaiClient::requestError, this, &SettingsViewBridge::onClientError);
    connect(&mClient, &KimaiClient::replyReceived, this, &SettingsViewBridge::onClientReply);
}

void SettingsViewBridge::setHost(const QString& host)
{
    if (host == mSettings.kimai.host)
        return;

    mSettings.kimai.host = host;
    emit hostChanged(mSettings.kimai.host);
}

QString SettingsViewBridge::host() const
{
    return mSettings.kimai.host;
}

void SettingsViewBridge::setUsername(const QString& username)
{
    if (username == mSettings.kimai.username)
        return;

    mSettings.kimai.username = username;
    emit usernameChanged(mSettings.kimai.username);
}

QString SettingsViewBridge::username() const
{
    return mSettings.kimai.username;
}

void SettingsViewBridge::setToken(const QString& token)
{
    if (token == mSettings.kimai.token)
        return;

    mSettings.kimai.token = token;
    emit tokenChanged(mSettings.kimai.token);
}

QString SettingsViewBridge::token() const
{
    return mSettings.kimai.token;
}

void SettingsViewBridge::test()
{
    mClient.setHost(mSettings.kimai.host);
    mClient.setUsername(mSettings.kimai.username);
    mClient.setToken(mSettings.kimai.token);
    mClient.sendRequest(KimaiRequestFactory::version());
}

void SettingsViewBridge::save()
{
    Settings::save(mSettings);
}

void SettingsViewBridge::onClientError(const QString& errorMsg)
{
    emit testFinished(false, errorMsg);
}

void SettingsViewBridge::onClientReply(const client::KimaiReply& reply)
{
    if (!reply.isValid())
    {
        emit testFinished(false, tr("Invalid reply."));
    }
    else
    {
        auto version = reply.get<KimaiVersion>();
        emit testFinished(true, tr("Connected to Kimai %1").arg(version.kimai.toString()));
    }
}
