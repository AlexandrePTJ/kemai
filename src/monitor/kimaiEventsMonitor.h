#pragma once

#include <memory>

#include <QObject>
#include <QTimer>

#include "client/kimaiClient.h"

namespace kemai {

class KimaiEventsMonitor : public QObject
{
    Q_OBJECT

public:
    KimaiEventsMonitor(std::shared_ptr<KimaiClient> kimaiClient);
    ~KimaiEventsMonitor() override;

    void refreshCurrentTimeSheet();
    void refreshRecentTimeSheets();

    std::optional<TimeSheet> currentTimeSheet() const;
    bool hasCurrentTimeSheet() const;

    std::vector<TimeSheet> recentTimeSheets() const;

signals:
    void currentTimeSheetChanged();
    void recentTimeSheetsChanged();

private:
    void onSecondTimeout();
    void onClientError(KimaiApiBaseResult* apiBaseResult);
    void onActiveTimeSheetsReceived(TimeSheetsResult timeSheetsResult);
    void onRecentTimeSheetsReceived(TimeSheetsResult timeSheetsResult);

    std::shared_ptr<KimaiClient> mKimaiClient;
    std::optional<TimeSheet> mCurrentTimeSheet;
    std::vector<TimeSheet> mRecentTimeSheets;
    std::optional<QDateTime> mLastTimeSheetUpdate;
    QTimer mSecondTimer;
};

} // namespace kemai
