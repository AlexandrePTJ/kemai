#include "activityWidget.h"
#include "ui_activityWidget.h"

#include <spdlog/spdlog.h>

#include "misc/customFmt.h"
#include "misc/helpers.h"
#include "timeSheetItemDelegate.h"
#include "timeSheetListWidgetItem.h"
#include "timeSheetParamsItemDelegate.h"

using namespace kemai;

static const auto gDefaultDurationText = "--:--:--";

ActivityWidget::ActivityWidget(QWidget* parent) : QWidget(parent), mUi(std::make_unique<Ui::ActivityWidget>())
{
    mUi->setupUi(this);
    mUi->lbDurationTime->setText(gDefaultDurationText);

    mUi->cbTimeSheetParams->setModel(&mTimeSheetParamsModel);
    mUi->cbTimeSheetParams->setItemDelegate(new TimeSheetParamsItemDelegate);

    mUi->lwHistory->setModel(&mTimeSheetModel);
    mUi->lwHistory->setItemDelegate(new TimeSheetItemDelegate);

    auto scrollBarWidth = qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    mUi->lwHistoryOutsideLayout->setContentsMargins(scrollBarWidth, 0, 0, 0);

    connect(mUi->btStartStop, &QPushButton::clicked, this, &ActivityWidget::onBtStartStopClicked);
    connect(mUi->cbTimeSheetParams, &QComboBox::currentIndexChanged, this, &ActivityWidget::onTimeSheetParamsIndexChanged);
    connect(&mSecondTimer, &QTimer::timeout, this, &ActivityWidget::onSecondTimeout);

    mSecondTimer.setInterval(std::chrono::seconds(1));
    mSecondTimer.setTimerType(Qt::PreciseTimer);
    mSecondTimer.start();

    updateControls();
}

ActivityWidget::~ActivityWidget()
{
    mSecondTimer.stop();
}

void ActivityWidget::setKemaiSession(const std::shared_ptr<KemaiSession>& kemaiSession)
{
    mSession = kemaiSession;

    mTimeSheetModel.setKemaiSession(kemaiSession);

    if (mSession)
    {
        connect(mSession.get(), &KemaiSession::currentTimeSheetChanged, this, &ActivityWidget::onSessionCurrentTimeSheetChanged);
        connect(&mSession->cache(), &KimaiCache::synchronizeStarted, this, [this]() { setEnabled(false); });
        connect(&mSession->cache(), &KimaiCache::synchronizeFinished, this, &ActivityWidget::onSessionCacheSynchronizeFinished);
    }
    else
    {
        setEnabled(false);
    }
}

void ActivityWidget::stopCurrentTimeSheet()
{
    if (mSession)
    {
        if (mSession->hasCurrentTimeSheet())
        {
            auto timeSheet = mSession->currentTimeSheet().value();

            timeSheet.endAt = mSession->computeTZDateTime(QDateTime::currentDateTime());

            auto timeSheetResult = mSession->client()->updateTimeSheet(timeSheet, mSession->timeSheetConfig().trackingMode);

            connect(timeSheetResult, &KimaiApiBaseResult::ready, this, [this, timeSheetResult] {
                if (mPendingStartRequest.has_value())
                {
                    startPendingTimeSheet();
                }
                else
                {
                    mSession->refreshCache(KimaiCache::Category::RecentTimeSheets);
                    mSession->refreshCurrentTimeSheet();
                }
                timeSheetResult->deleteLater();
            });
            connect(timeSheetResult, &KimaiApiBaseResult::error, [timeSheetResult]() { timeSheetResult->deleteLater(); });
        }
    }
}

void ActivityWidget::onSecondTimeout()
{
    if (mSession && mSession->hasCurrentTimeSheet())
    {
        mUi->lbDurationTime->setText(helpers::getDurationString(mSession->currentTimeSheet()->beginAt, QDateTime::currentDateTime()));
    }
}

void ActivityWidget::onSessionCurrentTimeSheetChanged()
{
    // Waiting for cache to be filled before trying to update combos
    while (mSession->cache().status() != KimaiCache::Status::Ready)
    {
        qApp->processEvents();
    }

    updateControls();
}

void ActivityWidget::onSessionCacheSynchronizeFinished()
{
    updateRecentTimeSheetsView();
    setEnabled(true);

    // Update all fields in case cache have refreshed with a running timesheet
    if (mSession->hasCurrentTimeSheet())
    {
        onSessionCurrentTimeSheetChanged();
    }

    mTimeSheetParamsModel.updateDataFromCache(mSession->cache());
    mUi->cbTimeSheetParams->setCurrentIndex(-1);
}

void ActivityWidget::onHistoryTimeSheetStartRequested(const TimeSheet& timeSheet)
{
    mPendingStartRequest = timeSheet;

    if (mSession->hasCurrentTimeSheet())
    {
        stopCurrentTimeSheet();
    }
    else
    {
        fillFromTimesheet(timeSheet);
        startPendingTimeSheet();
    }
}

void ActivityWidget::onHistoryTimeSheetFillRequested(const TimeSheet& timeSheet)
{
    if (mSession->hasCurrentTimeSheet())
    {
        stopCurrentTimeSheet();
    }
    fillFromTimesheet(timeSheet);
}

void ActivityWidget::fillFromTimesheet(const TimeSheet& timeSheet)
{
    auto index = mTimeSheetParamsModel.findIndex(timeSheet.project.id, timeSheet.activity.id);
    mUi->cbTimeSheetParams->setCurrentIndex(index);
}

void ActivityWidget::onBtStartStopClicked()
{
    if (mSession->hasCurrentTimeSheet())
    {
        stopCurrentTimeSheet();
    }
    else if (isCurrentTimeSheetParamsValid())
    {
        TimeSheet timeSheet;
        std::tie(timeSheet.project.id, timeSheet.activity.id) = getCurrentTimeSheetParams();
        timeSheet.beginAt                                     = mSession->computeTZDateTime(QDateTime::currentDateTime());

        auto timeSheetResult = mSession->client()->startTimeSheet(timeSheet, mSession->timeSheetConfig().trackingMode);

        connect(timeSheetResult, &KimaiApiBaseResult::ready, this, [this, timeSheetResult] {
            mSession->refreshCurrentTimeSheet();
            timeSheetResult->deleteLater();
        });
        connect(timeSheetResult, &KimaiApiBaseResult::error, [timeSheetResult]() { timeSheetResult->deleteLater(); });
    }
}

void ActivityWidget::onTimeSheetParamsIndexChanged(int /*index*/)
{
    mUi->btStartStop->setEnabled(isCurrentTimeSheetParamsValid());
}

void ActivityWidget::updateControls()
{
    // Can be raised while clearing combobox on session changes
    if (!mSession)
    {
        setEnabled(false);
        return;
    }

    mUi->btStartStop->setEnabled(mPendingStartRequest.has_value() || mSession->hasCurrentTimeSheet());

    if (mSession->hasCurrentTimeSheet())
    {
        mUi->btStartStop->setIcon(QIcon(":/icons/stop"));
    }
    else
    {
        mUi->btStartStop->setIcon(QIcon(":/icons/play"));
        mUi->lbDurationTime->setText(gDefaultDurationText);
    }
}

void ActivityWidget::updateRecentTimeSheetsView()
{
    //    mUi->lwHistory->clear();
    //    for (const auto& timeSheet : mSession->cache().recentTimeSheets())
    //    {
    //        auto timeSheetListWidgetItem = new TimeSheetListWidgetItem(timeSheet);
    //
    //        auto item = new QListWidgetItem;
    //        item->setSizeHint(timeSheetListWidgetItem->sizeHint());
    //
    //        mUi->lwHistory->addItem(item);
    //        mUi->lwHistory->setItemWidget(item, timeSheetListWidgetItem);
    //
    //        connect(timeSheetListWidgetItem, &TimeSheetListWidgetItem::timeSheetStartRequested, this, &ActivityWidget::onHistoryTimeSheetStartRequested);
    //        connect(timeSheetListWidgetItem, &TimeSheetListWidgetItem::timeSheetFillRequested, this, &ActivityWidget::onHistoryTimeSheetFillRequested);
    //    }
}

void ActivityWidget::startPendingTimeSheet()
{
    if (mPendingStartRequest.has_value())
    {
        TimeSheet timeSheet;

        timeSheet.beginAt     = mSession->computeTZDateTime(QDateTime::currentDateTime());
        timeSheet.project.id  = mPendingStartRequest->project.id;
        timeSheet.activity.id = mPendingStartRequest->activity.id;

        mPendingStartRequest.reset();

        auto timeSheetResult = mSession->client()->startTimeSheet(timeSheet, mSession->timeSheetConfig().trackingMode);

        connect(timeSheetResult, &KimaiApiBaseResult::ready, this, [this, timeSheetResult] {
            mSession->refreshCurrentTimeSheet();
            timeSheetResult->deleteLater();
        });
        connect(timeSheetResult, &KimaiApiBaseResult::error, [timeSheetResult]() { timeSheetResult->deleteLater(); });
    }
}

bool ActivityWidget::isCurrentTimeSheetParamsValid() const
{
    return mUi->cbTimeSheetParams->currentData(TimeSheetParamsModel::ProjectIdRole).isValid() &&
           mUi->cbTimeSheetParams->currentData(TimeSheetParamsModel::ActivityIdRole).isValid();
}

std::pair<int, int> ActivityWidget::getCurrentTimeSheetParams() const
{
    return {mUi->cbTimeSheetParams->currentData(TimeSheetParamsModel::ProjectIdRole).toInt(),
            mUi->cbTimeSheetParams->currentData(TimeSheetParamsModel::ActivityIdRole).toInt()};
}
