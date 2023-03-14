#include "macDesktopEventsMonitor.h"

#import <CoreGraphics/CoreGraphics.h>

using namespace kemai;

MacDesktopEventsMonitor::MacDesktopEventsMonitor() : DesktopEventsMonitor(false, true)
{
    connect(&mPollTimer, &QTimer::timeout, this, &MacDesktopEventsMonitor::onPollTimeout);
}

void MacDesktopEventsMonitor::initialize(const Settings::Events& eventsSettings)
{
    stop();

    mEventsSettings = eventsSettings;
}

void MacDesktopEventsMonitor::start()
{
    if (mEventsSettings.stopOnIdle)
    {
        mPollTimer.start(std::chrono::seconds(1));
    }
}

void MacDesktopEventsMonitor::stop()
{
    mPollTimer.stop();
}

void MacDesktopEventsMonitor::onPollTimeout()
{
    auto interval = CGEventSourceSecondsSinceLastEventType(CGEventSourceStateID::kCGEventSourceStateCombinedSessionState, kCGAnyInputEventType);

    auto idleSinceNSecs = std::chrono::seconds(static_cast<int>(interval));

    if (mEventsSettings.stopOnIdle && mEventsSettings.idleDelayMinutes <= std::chrono::duration_cast<std::chrono::minutes>(idleSinceNSecs).count())
    {
        emit idleDetected();
    }
}
