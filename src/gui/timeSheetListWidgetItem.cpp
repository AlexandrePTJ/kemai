#include "timeSheetListWidgetItem.h"
#include "ui_timeSheetListWidgetItem.h"

#include <misc/helpers.h>

using namespace kemai;

TimeSheetListWidgetItem::TimeSheetListWidgetItem(const TimeSheet& timeSheet, QWidget* parent)
    : QWidget(parent), mTimeSheet(timeSheet), mUi(new Ui::TimeSheetListWidgetItem)
{
    mUi->setupUi(this);
    mUi->frame->setFrameShape(QFrame::StyledPanel);
    mUi->lbActivity->setText(mTimeSheet.activity.name);
    mUi->lbDescription->setText(mTimeSheet.project.name);
    mUi->lbDuration->setText(helpers::getDurationString(timeSheet.beginAt, timeSheet.endAt));
    mUi->lbStartedAt->setText(timeSheet.beginAt.toString(Qt::ISODate));

    connect(mUi->btStart, &QPushButton::clicked, [this]() { emit timeSheetStartRequested(mTimeSheet); });
}

TimeSheetListWidgetItem::~TimeSheetListWidgetItem()
{
    delete mUi;
}
