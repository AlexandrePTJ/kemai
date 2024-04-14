#include "kemaiSession.h"

#include <spdlog/spdlog.h>

#include <QTimeZone>

#include "client/kimaiFeatures.h"
#include "misc/customFmt.h"

using namespace kemai;

/*
 * Class impl
 */
KemaiSession::KemaiSession(const std::shared_ptr<KimaiClient>& kimaiClient) : mKimaiClient(kimaiClient), mKimaiMonitor(kimaiClient)
{
    connect(&mKimaiMonitor, &KimaiEventsMonitor::currentTimeSheetChanged, this, &KemaiSession::currentTimeSheetChanged);
}

KemaiSession::~KemaiSession() = default;

std::shared_ptr<KimaiClient> KemaiSession::client() const
{
    return mKimaiClient;
}

const KimaiCache& KemaiSession::cache() const
{
    return mKimaiCache;
}

void KemaiSession::refreshSessionInfos()
{
    requestMe();
    requestVersion();
    requestTimeSheetConfig();
}

void KemaiSession::refreshCurrentTimeSheet()
{
    mKimaiMonitor.refreshCurrentTimeSheet();
}

void KemaiSession::refreshCache(KimaiCache::Category category)
{
    if (mKimaiClient)
    {
        mKimaiCache.synchronize(mKimaiClient, {category});
    }
}

void KemaiSession::refreshCache(const std::set<KimaiCache::Category>& categories)
{
    if (mKimaiClient)
    {
        mKimaiCache.synchronize(mKimaiClient, categories);
    }
}

bool KemaiSession::hasPlugin(ApiPlugin apiPlugin) const
{
    return std::any_of(mPlugins.begin(), mPlugins.end(), [apiPlugin](const Plugin& plugin) { return plugin.apiPlugin == apiPlugin; });
}

QVersionNumber KemaiSession::kimaiVersion() const
{
    return mKimaiVersion;
}

User KemaiSession::me() const
{
    return mMe;
}

TimeSheetConfig KemaiSession::timeSheetConfig() const
{
    return mTimeSheetConfig;
}

std::optional<TimeSheet> KemaiSession::currentTimeSheet() const
{
    return mKimaiMonitor.currentTimeSheet();
}

bool KemaiSession::hasCurrentTimeSheet() const
{
    return mKimaiMonitor.hasCurrentTimeSheet();
}

QDateTime KemaiSession::computeTZDateTime(const QDateTime& dateTime) const
{
    // Be sure to use expected timezone
    auto timeZone = QTimeZone(mMe.timezone.toLocal8Bit());
    if (timeZone.isValid())
    {
        return dateTime.toTimeZone(timeZone);
    }
    return dateTime;
}

void KemaiSession::onClientError(KimaiApiBaseResult* apiBaseResult)
{
    spdlog::error("Client error: {}", apiBaseResult->errorMessage());
    apiBaseResult->deleteLater();
}

void KemaiSession::requestMe()
{
    auto meResult = mKimaiClient->requestMeUserInfo();

    connect(meResult, &KimaiApiBaseResult::ready, this, [this, meResult]() {
        mMe = meResult->getResult();
        emit meChanged();
        meResult->deleteLater();
    });

    connect(meResult, &KimaiApiBaseResult::error, this, [this, meResult]() { onClientError(meResult); });
}

void KemaiSession::requestVersion()
{
    auto versionResult = mKimaiClient->requestKimaiVersion();

    connect(versionResult, &KimaiApiBaseResult::ready, this, [this, versionResult]() {
        mKimaiVersion = versionResult->getResult().kimai;
        emit versionChanged();
        if (KimaiFeatures::canRequestPlugins(mKimaiVersion))
        {
            requestPlugins();
        }
        versionResult->deleteLater();
    });

    connect(versionResult, &KimaiApiBaseResult::error, this, [this, versionResult]() { onClientError(versionResult); });
}

void KemaiSession::requestTimeSheetConfig()
{
    auto timeSheetConfigResult = mKimaiClient->requestTimeSheetConfig();

    connect(timeSheetConfigResult, &KimaiApiBaseResult::ready, this, [this, timeSheetConfigResult]() {
        mTimeSheetConfig = timeSheetConfigResult->getResult();
        emit timeSheetConfigChanged();
        timeSheetConfigResult->deleteLater();
    });

    connect(timeSheetConfigResult, &KimaiApiBaseResult::error, this, [this, timeSheetConfigResult]() { onClientError(timeSheetConfigResult); });
}

void KemaiSession::requestPlugins()
{
    auto pluginsResult = mKimaiClient->requestPlugins();

    connect(pluginsResult, &KimaiApiBaseResult::ready, this, [this, pluginsResult]() {
        mPlugins = pluginsResult->getResult();
        emit pluginsChanged();
        pluginsResult->deleteLater();
    });

    connect(pluginsResult, &KimaiApiBaseResult::error, this, [this, pluginsResult]() { onClientError(pluginsResult); });
}
