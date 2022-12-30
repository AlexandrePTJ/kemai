#include "desktopEventsMonitor.h"

#ifdef Q_OS_WINDOWS
#    include "windowsDesktopEventsMonitor.h"
#endif

using namespace kemai;

std::shared_ptr<DesktopEventsMonitor> DesktopEventsMonitor::create()
{
#ifdef Q_OS_WINDOWS
    return std::make_shared<WindowsDesktopEventsMonitor>();
#else
    return {};
#endif // Q_OS_WINDOWS
}

bool DesktopEventsMonitor::hasLockSupport() const
{
    return mHasLockSupport;
}

bool DesktopEventsMonitor::hasIdleSupport() const
{
    return mHasIdleSupport;
}
