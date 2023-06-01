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

    void refreshCurrentTimeSheets();

    std::optional<TimeSheet> currentTimeSheet() const;
    TimeSheets currentTimeSheets() const;

    bool hasCurrentTimeSheet() const;

signals:
    void currentTimeSheetsChanged();

private:
    void onSecondTimeout();
    void onClientError(KimaiApiBaseResult* apiBaseResult);
    void onActiveTimeSheetsReceived(TimeSheetsResult timeSheetsResult);

    std::shared_ptr<KimaiClient> mKimaiClient;
    TimeSheets mCurrentTimeSheets;
    std::optional<QDateTime> mLastTimeSheetUpdate;
    QTimer mSecondTimer;
};

} // namespace kemai
