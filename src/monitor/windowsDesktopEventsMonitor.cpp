#include "windowsDesktopEventsMonitor.h"

#include <spdlog/spdlog.h>

#include <Windows.h>

using namespace kemai;

WindowsDesktopEventsMonitor::WindowsDesktopEventsMonitor()
{
    mHasLockSupport = false;
    mHasIdleSupport = true;

    connect(&mPollTimer, &QTimer::timeout, this, &WindowsDesktopEventsMonitor::onPollTimeout);
}

void WindowsDesktopEventsMonitor::initialize(const Settings::Events& eventsSettings)
{
    mEventsSettings = eventsSettings;

    if (eventsSettings.stopOnIdle)
    {
        mPollTimer.start(std::chrono::seconds(1));
    }
    else
    {
        mPollTimer.stop();
    }
}

void WindowsDesktopEventsMonitor::onPollTimeout()
{
    LASTINPUTINFO lastInputInfo;
    lastInputInfo.cbSize = sizeof(LASTINPUTINFO);

    GetLastInputInfo(&lastInputInfo);
    auto idleSinceNMilliSecs = std::chrono::milliseconds(GetTickCount() - lastInputInfo.dwTime);
    
    if (mEventsSettings.stopOnIdle && mEventsSettings.idleDelayMinutes <= std::chrono::duration_cast<std::chrono::minutes>(idleSinceNMilliSecs).count())
    {
        emit idleDetected();
    }
}
