#pragma once

#include <QTimer>
#include <QWidget>

#include "client/kimaiClient.h"
#include "context/kemaiSession.h"
#include "models/timeSheetParamsModel.h"

namespace Ui {
class ActivityWidget;
}

namespace kemai {

class ActivityWidget : public QWidget
{
    Q_OBJECT

public:
    ActivityWidget(QWidget* parent = nullptr);
    ~ActivityWidget() override;

    void setKemaiSession(std::shared_ptr<KemaiSession> kemaiSession);
    void stopCurrentTimeSheet();

private:
    void onBtStartStopClicked();
    void onTimeSheetParamsIndexChanged(int index);

    void onSecondTimeout();

    void onSessionCurrentTimeSheetChanged();
    void onSessionCacheSynchronizeFinished();

    void onHistoryTimeSheetStartRequested(const TimeSheet& timeSheet);
    void onHistoryTimeSheetFillRequested(const TimeSheet& timeSheet);
    void fillFromTimesheet(const TimeSheet& timeSheet);

    void updateControls();

    void updateRecentTimeSheetsView();

    void startPendingTimeSheet();

    bool isCurrentTimeSheetParamsValid() const;
    std::pair<int, int> getCurrentTimeSheetParams() const;

    std::unique_ptr<Ui::ActivityWidget> mUi;
    QTimer mSecondTimer;
    std::shared_ptr<KemaiSession> mSession;
    std::optional<TimeSheet> mPendingStartRequest;
    TimeSheetParamsModel mTimeSheetParamsModel;
};

} // namespace kemai
