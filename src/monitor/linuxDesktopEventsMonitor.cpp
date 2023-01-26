#include "linuxDesktopEventsMonitor.h"

#include <X11/extensions/scrnsaver.h>

#include <spdlog/spdlog.h>

using namespace kemai;

LinuxDesktopEventsMonitor::LinuxDesktopEventsMonitor() : DesktopEventsMonitor(false, true)
{
    connect(&mPollTimer, &QTimer::timeout, this, &LinuxDesktopEventsMonitor::onPollTimeout);
}

void LinuxDesktopEventsMonitor::initialize(const Settings::Events& eventsSettings)
{
    stop();

    mEventsSettings = eventsSettings;
}

void LinuxDesktopEventsMonitor::start()
{
    if (mEventsSettings.stopOnIdle)
    {
        mPollTimer.start(std::chrono::seconds(1));
    }
}

void LinuxDesktopEventsMonitor::stop()
{
    mPollTimer.stop();
}

void LinuxDesktopEventsMonitor::onPollTimeout()
{
    auto x11Display = XOpenDisplay(nullptr);

    if (x11Display == nullptr)
    {
        spdlog::error("Cannot open display to check idle");
        return;
    }

    auto xssInfo = XScreenSaverAllocInfo();
    XScreenSaverQueryInfo(x11Display, DefaultRootWindow(x11Display), xssInfo);

    auto idleSinceNMilliSecs = std::chrono::milliseconds(xssInfo->idle);
    if (mEventsSettings.stopOnIdle && mEventsSettings.idleDelayMinutes <= std::chrono::duration_cast<std::chrono::minutes>(idleSinceNMilliSecs).count())
    {
        emit idleDetected();
    }

    XCloseDisplay(x11Display);
}
