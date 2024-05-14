#include "timeSheetListWidgetItem.h"
#include "ui_timeSheetListWidgetItem.h"

#include <misc/helpers.h>

using namespace kemai;

TimeSheetListWidgetItem::TimeSheetListWidgetItem(const TimeSheet& timeSheet, QWidget* parent)
    : QWidget(parent), mTimeSheet(timeSheet), mUi(std::make_unique<Ui::TimeSheetListWidgetItem>())
{
    mUi->setupUi(this);

    mUi->lbActivity->setText(mTimeSheet.activity.name);

    auto color = timeSheet.project.color.isEmpty() ? timeSheet.project.customer.color : timeSheet.project.color;
    mUi->lbProject->setText(timeSheet.project.name);
    mUi->lbProject->setStyleSheet(QString("QLabel { color : %1; }").arg(color));

    mUi->lbDuration->setText(helpers::getDurationString(timeSheet.beginAt, timeSheet.endAt));
    mUi->lbTimestamps->setText(timeSheet.beginAt.toString(Qt::ISODate));

    connect(mUi->btStart, &QPushButton::clicked, [this]() { emit timeSheetStartRequested(mTimeSheet); });
    connect(mUi->btFill, &QPushButton::clicked, [this]() { emit timeSheetFillRequested(mTimeSheet); });
}

TimeSheetListWidgetItem::~TimeSheetListWidgetItem() = default;
