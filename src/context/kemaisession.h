#pragma once

#include <optional>

#include <QObject>

#include "client/kimaiCache.h"
#include "client/kimaiapi.h"
#include "client/kimaiclient.h"

namespace kemai {

class KemaiSession : public QObject
{
    Q_OBJECT

public:
    KemaiSession(std::shared_ptr<KimaiClient> kimaiClient);
    ~KemaiSession() override;

    std::shared_ptr<KimaiClient> client() const;
    const KimaiCache& cache() const;

    /*
     * send some request to identify instance
     */
    void refreshSessionInfos();
    void refreshCurrentTimeSheet();
    void refreshCache(const std::set<KimaiCache::Category>& categories = {});

    bool hasPlugin(ApiPlugin apiPlugin) const;
    User me() const;
    TimeSheetConfig timeSheetConfig() const;

    std::optional<TimeSheet> currentTimeSheet() const;
    bool hasCurrentTimeSheet() const;
    QDateTime computeTZDateTime(const QDateTime& dateTime) const;

signals:
    void pluginsChanged();
    void currentTimeSheetChanged();

private:
    void onClientError(KimaiApiBaseResult* apiBaseResult);

    void requestMe();
    void requestVersion();
    void requestTimeSheetConfig();
    void requestPlugins();

    void setCurrentTimeSheet(const TimeSheet& timeSheet);
    void clearCurrentTimeSheet();

    std::shared_ptr<KimaiClient> mKimaiClient;
    KimaiCache mKimaiCache;
    QVersionNumber mKimaiVersion;
    Plugins mPlugins;
    User mMe;
    TimeSheetConfig mTimeSheetConfig;
    std::optional<TimeSheet> mCurrentTimeSheet;
};

} // namespace kemai
