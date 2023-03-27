#include "kimaiEventsMonitor.h"

#include <spdlog/spdlog.h>

#include "settings/settings.h"

using namespace kemai;

KimaiEventsMonitor::KimaiEventsMonitor(std::shared_ptr<KimaiClient> kimaiClient) : mKimaiClient(std::move(kimaiClient))
{
    connect(&mSecondTimer, &QTimer::timeout, this, &KimaiEventsMonitor::onSecondTimeout);

    mSecondTimer.setInterval(std::chrono::seconds(1));
    mSecondTimer.setTimerType(Qt::PreciseTimer);
    mSecondTimer.start();
}

KimaiEventsMonitor::~KimaiEventsMonitor() = default;

void KimaiEventsMonitor::refreshCurrentTimeSheet()
{
    auto activeTimeSheetsResult = mKimaiClient->requestActiveTimeSheets();
    connect(activeTimeSheetsResult, &KimaiApiBaseResult::ready, this, [this, activeTimeSheetsResult] { onActiveTimeSheetsReceived(activeTimeSheetsResult); });
    connect(activeTimeSheetsResult, &KimaiApiBaseResult::error, this, [this, activeTimeSheetsResult]() { onClientError(activeTimeSheetsResult); });
}

std::optional<TimeSheet> KimaiEventsMonitor::currentTimeSheet() const
{
    return mCurrentTimeSheet;
}

bool KimaiEventsMonitor::hasCurrentTimeSheet() const
{
    return mCurrentTimeSheet.has_value();
}

void KimaiEventsMonitor::onSecondTimeout()
{
    auto settings = Settings::get();
    if (settings.events.autoRefreshCurrentTimeSheet && mLastTimeSheetUpdate.has_value())
    {
        if (mLastTimeSheetUpdate->secsTo(QDateTime::currentDateTime()) >= settings.events.autoRefreshCurrentTimeSheetDelaySeconds)
        {
            refreshCurrentTimeSheet();
        }
    }
}

void KimaiEventsMonitor::onClientError(KimaiApiBaseResult* apiBaseResult)
{
    spdlog::error("Client error: {}", apiBaseResult->errorMessage().toStdString());
    apiBaseResult->deleteLater();
}

void KimaiEventsMonitor::onActiveTimeSheetsReceived(TimeSheetsResult timeSheetsResult)
{
    const auto& timeSheets = timeSheetsResult->getResult();

    bool firstRun  = !mLastTimeSheetUpdate.has_value();
    bool isRunning = mCurrentTimeSheet.has_value();

    if(timeSheets.empty())
    {
        if (isRunning || firstRun)
        {
            mCurrentTimeSheet.reset();
            emit currentTimeSheetChanged();
        }
    }
    else
    {
        auto timeSheet = timeSheets.front();
        bool isSame    = isRunning && timeSheet.id == mCurrentTimeSheet->id;

        if (!isSame || firstRun || !isRunning)
        {
            mCurrentTimeSheet = timeSheet;
            emit currentTimeSheetChanged();
        }
    }

    mLastTimeSheetUpdate = QDateTime::currentDateTime();
    timeSheetsResult->deleteLater();
}
