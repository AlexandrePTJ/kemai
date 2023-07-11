#pragma once

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

signals:
    void timeSheetStartRequested(const TimeSheet& timeSheet);

private:
    const TimeSheet mTimeSheet;
    Ui::TimeSheetListWidgetItem* mUi;
};

} // namespace kemai
