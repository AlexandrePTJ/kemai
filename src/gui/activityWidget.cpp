#include "activityWidget.h"
#include "ui_activityWidget.h"

#include <spdlog/spdlog.h>

#include "misc/customFmt.h"
#include "misc/helpers.h"
#include "timeSheetListWidgetItem.h"
#include "timeSheetParamsItemDelegate.h"

using namespace kemai;

ActivityWidget::ActivityWidget(QWidget* parent) : QWidget(parent), mUi(std::make_unique<Ui::ActivityWidget>())
{
    mUi->setupUi(this);

    mUi->cbTimeSheetParams->setModel(&mTimeSheetParamsModel);
    mUi->cbTimeSheetParams->setItemDelegate(new TimeSheetParamsItemDelegate);

    connect(mUi->btStartStop, &QPushButton::clicked, this, &ActivityWidget::onBtStartStopClicked);
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

void ActivityWidget::setKemaiSession(std::shared_ptr<KemaiSession> kemaiSession)
{
    mSession = std::move(kemaiSession);

    mUi->lwHistory->clear();

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

void ActivityWidget::fillFromTimesheet(const TimeSheet& timeSheet) {}

void ActivityWidget::onBtStartStopClicked()
{
    if (mSession->hasCurrentTimeSheet())
    {
        stopCurrentTimeSheet();
    }
    else
    {
        TimeSheet timeSheet;

        auto timeSheetResult = mSession->client()->startTimeSheet(timeSheet, mSession->timeSheetConfig().trackingMode);

        connect(timeSheetResult, &KimaiApiBaseResult::ready, this, [this, timeSheetResult] {
            mSession->refreshCurrentTimeSheet();
            timeSheetResult->deleteLater();
        });
        connect(timeSheetResult, &KimaiApiBaseResult::error, [timeSheetResult]() { timeSheetResult->deleteLater(); });
    }
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
        mUi->lbDurationTime->clear();
    }
}

void ActivityWidget::updateRecentTimeSheetsView()
{
    mUi->lwHistory->clear();
    for (const auto& timeSheet : mSession->cache().recentTimeSheets())
    {
        auto timeSheetListWidgetItem = new TimeSheetListWidgetItem(timeSheet);

        auto item = new QListWidgetItem;
        item->setSizeHint(timeSheetListWidgetItem->sizeHint());

        mUi->lwHistory->addItem(item);
        mUi->lwHistory->setItemWidget(item, timeSheetListWidgetItem);

        connect(timeSheetListWidgetItem, &TimeSheetListWidgetItem::timeSheetStartRequested, this, &ActivityWidget::onHistoryTimeSheetStartRequested);
        connect(timeSheetListWidgetItem, &TimeSheetListWidgetItem::timeSheetFillRequested, this, &ActivityWidget::onHistoryTimeSheetFillRequested);
    }
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
