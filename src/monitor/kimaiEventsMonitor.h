#pragma once

#include <memory>

#include <QObject>
#include <QTimer>

#include "client/kimaiclient.h"

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

signals:
    void currentTimeSheetChanged();

private:
    void onSecondTimeout();
    void onClientError(KimaiApiBaseResult* apiBaseResult);
    void onActiveTimeSheetsReceived(TimeSheetsResult timeSheetsResult);

    std::shared_ptr<KimaiClient> mKimaiClient;
    std::optional<TimeSheet> mCurrentTimeSheet;
    std::optional<QDateTime> mLastTimeSheetUpdate;
    QTimer mSecondTimer;
};

} // namespace kemai
