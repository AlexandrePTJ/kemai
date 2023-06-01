#include "kimaiEventsMonitor.h"

#include <mutex>
#include <set>

#include <spdlog/spdlog.h>

#include "settings/settings.h"

using namespace kemai;

static std::mutex gTimeSheetsMutex;

KimaiEventsMonitor::KimaiEventsMonitor(std::shared_ptr<KimaiClient> kimaiClient) : mKimaiClient(std::move(kimaiClient))
{
    connect(&mSecondTimer, &QTimer::timeout, this, &KimaiEventsMonitor::onSecondTimeout);

    mSecondTimer.setInterval(std::chrono::seconds(1));
    mSecondTimer.setTimerType(Qt::PreciseTimer);
    mSecondTimer.start();
}

KimaiEventsMonitor::~KimaiEventsMonitor() = default;

void KimaiEventsMonitor::refreshCurrentTimeSheets()
{
    auto activeTimeSheetsResult = mKimaiClient->requestActiveTimeSheets();
    connect(activeTimeSheetsResult, &KimaiApiBaseResult::ready, this, [this, activeTimeSheetsResult] { onActiveTimeSheetsReceived(activeTimeSheetsResult); });
    connect(activeTimeSheetsResult, &KimaiApiBaseResult::error, this, [this, activeTimeSheetsResult]() { onClientError(activeTimeSheetsResult); });
}

std::optional<TimeSheet> KimaiEventsMonitor::currentTimeSheet() const
{
    const std::scoped_lock<std::mutex> lock(gTimeSheetsMutex);
    return mCurrentTimeSheets.empty() ? std::optional<TimeSheet>{} : mCurrentTimeSheets.front();
}

TimeSheets KimaiEventsMonitor::currentTimeSheets() const
{
    const std::scoped_lock<std::mutex> lock(gTimeSheetsMutex);
    return mCurrentTimeSheets;
}

bool KimaiEventsMonitor::hasCurrentTimeSheet() const
{
    return !mCurrentTimeSheets.empty();
}

void KimaiEventsMonitor::onSecondTimeout()
{
    auto settings = Settings::get();
    if (settings.events.autoRefreshCurrentTimeSheet && mLastTimeSheetUpdate.has_value())
    {
        if (mLastTimeSheetUpdate->secsTo(QDateTime::currentDateTime()) >= settings.events.autoRefreshCurrentTimeSheetDelaySeconds)
        {
            refreshCurrentTimeSheets();
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
    const std::scoped_lock<std::mutex> lock(gTimeSheetsMutex);
    auto timeSheets = timeSheetsResult->getResult();

    bool firstRun  = !mLastTimeSheetUpdate.has_value();
    bool isRunning = !mCurrentTimeSheets.empty();

    if (timeSheets.empty())
    {
        if (isRunning || firstRun)
        {
            mCurrentTimeSheets.clear();
            emit currentTimeSheetsChanged();
        }
    }
    else
    {
        /*
         * Order may differ, compare list of active timesheet ids
         */
        std::set<int> currentIds;
        std::set<int> newIds;
        std::for_each(mCurrentTimeSheets.begin(), mCurrentTimeSheets.end(), [&currentIds](const auto& timeSheet) { currentIds.insert(timeSheet.id); });
        std::for_each(timeSheets.begin(), timeSheets.end(), [&newIds](const auto& timeSheet) { newIds.insert(timeSheet.id); });

        bool isSame = isRunning && currentIds == newIds;

        if (!isSame || firstRun || !isRunning)
        {
            mCurrentTimeSheets = std::move(timeSheets);
            emit currentTimeSheetsChanged();
        }
    }

    mLastTimeSheetUpdate = QDateTime::currentDateTime();
    timeSheetsResult->deleteLater();
}
