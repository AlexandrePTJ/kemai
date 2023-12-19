#pragma once

#include <memory>

#include <QWidget>

#include "client/kimaiAPI.h"

namespace Ui {
class TimeSheetListWidgetItem;
}

namespace kemai {

class TimeSheetListWidgetItem : public QWidget
{
    Q_OBJECT

public:
    TimeSheetListWidgetItem(const TimeSheet& timeSheet, QWidget* parent = nullptr);
    ~TimeSheetListWidgetItem() override;

    void setIsActive(bool active);
    void updateDuration();

signals:
    void timeSheetStartRequested(const TimeSheet& timeSheet);
    void timeSheetStopRequested(const TimeSheet& timeSheet);

private:
    void onBtStartStopClicked();

    std::unique_ptr<Ui::TimeSheetListWidgetItem> mUi;
    const TimeSheet mTimeSheet;
    bool mIsActive = false;
};

} // namespace kemai
