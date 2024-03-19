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

signals:
    void timeSheetStartRequested(const TimeSheet& timeSheet);
    void timeSheetFillRequested(const TimeSheet& timeSheet);

private:
    std::unique_ptr<Ui::TimeSheetListWidgetItem> mUi;
    const TimeSheet mTimeSheet;
};

} // namespace kemai
