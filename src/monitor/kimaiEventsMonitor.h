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

    std::optional<TimeSheet> currentTimeSheet() const;
    bool hasCurrentTimeSheet() const;
    const std::vector<TimeSheet>& currentTimeSheets() const;

signals:
    void currentTimeSheetChanged();
    void currentTimeSheetsChanged();

private:
    void onSecondTimeout();
    void onClientError(KimaiApiBaseResult* apiBaseResult);
    void onActiveTimeSheetsReceived(TimeSheetsResult timeSheetsResult);

    std::shared_ptr<KimaiClient> mKimaiClient;
    std::vector<TimeSheet> mCurrentTimeSheets;
    std::optional<TimeSheet> mCurrentTimeSheet;
    std::optional<QDateTime> mLastTimeSheetUpdate;
    QTimer mSecondTimer;
};

} // namespace kemai
