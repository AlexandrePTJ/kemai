#include "timeSheetListWidgetItem.h"
#include "ui_timeSheetListWidgetItem.h"

#include <misc/helpers.h>

using namespace kemai;

TimeSheetListWidgetItem::TimeSheetListWidgetItem(const TimeSheet& timeSheet, QWidget* parent)
    : QWidget(parent), mTimeSheet(timeSheet), mUi(std::make_unique<Ui::TimeSheetListWidgetItem>())
{
    mUi->setupUi(this);
    mUi->frame->setFrameShape(QFrame::StyledPanel);
    mUi->lbActivity->setText(mTimeSheet.activity.name);
    mUi->lbDescription->setText(mTimeSheet.project.name);
    mUi->lbDuration->setText(helpers::getDurationString(timeSheet.beginAt, timeSheet.endAt));
    mUi->lbStartedAt->setText(timeSheet.beginAt.toString(Qt::RFC2822Date));

    connect(mUi->btStart, &QPushButton::clicked, this, &TimeSheetListWidgetItem::onBtStartStopClicked);
}

TimeSheetListWidgetItem::~TimeSheetListWidgetItem() = default;

void TimeSheetListWidgetItem::setIsActive(bool active)
{
    mIsActive = active;
    mUi->btStart->setIcon(QIcon(mIsActive ? ":/icons/stop" : ":/icons/play"));
}

void TimeSheetListWidgetItem::updateDuration()
{
    if (mIsActive)
    {
        mUi->lbDuration->setText(helpers::getDurationString(mTimeSheet.beginAt, QDateTime::currentDateTime()));
    }
}

void TimeSheetListWidgetItem::onBtStartStopClicked()
{
    if (mIsActive)
    {
        emit timeSheetStopRequested(mTimeSheet);
    }
    else
    {
        TimeSheet timeSheet;
        timeSheet.project.id  = mTimeSheet.project.id;
        timeSheet.activity.id = mTimeSheet.activity.id;

        emit timeSheetStartRequested(timeSheet);
    }
}
