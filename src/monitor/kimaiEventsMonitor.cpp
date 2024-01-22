#include "kimaiEventsMonitor.h"

#include <spdlog/spdlog.h>

#include "misc/customFmt.h"
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

const std::vector<TimeSheet>& KimaiEventsMonitor::currentTimeSheets() const
{
    return mCurrentTimeSheets;
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
    spdlog::error("Client error: {}", apiBaseResult->errorMessage());
    apiBaseResult->deleteLater();
}

void KimaiEventsMonitor::onActiveTimeSheetsReceived(TimeSheetsResult timeSheetsResult)
{
    auto timeSheets = timeSheetsResult->takeResult();

    bool firstRun  = !mLastTimeSheetUpdate.has_value();
    bool isRunning = mCurrentTimeSheet.has_value();

    if (timeSheets.empty())
    {
        if (isRunning || firstRun || !mCurrentTimeSheets.empty())
        {
            mCurrentTimeSheets.clear();
            mCurrentTimeSheet.reset();
            emit currentTimeSheetChanged();
            emit currentTimeSheetsChanged();
        }
    }
    else
    {
        std::sort(timeSheets.begin(), timeSheets.end(), [](const auto& a, const auto& b) { return a.beginAt < b.beginAt; });
        if (timeSheets.size() != mCurrentTimeSheets.size() ||
            !std::equal(timeSheets.begin(), timeSheets.end(), mCurrentTimeSheets.begin(), [](const auto& a, const auto& b) { return a.id == b.id; }))
        {
            mCurrentTimeSheets = timeSheets;
            emit currentTimeSheetsChanged();
        }

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
