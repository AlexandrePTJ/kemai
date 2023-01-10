#include "desktopEventsMonitor.h"

#ifdef Q_OS_WINDOWS
#    include "windowsDesktopEventsMonitor.h"
#elif defined Q_OS_MAC
#    include "macDesktopEventsMonitor.h"
#endif

using namespace kemai;

std::shared_ptr<DesktopEventsMonitor> DesktopEventsMonitor::create()
{
#ifdef Q_OS_WINDOWS
    return std::make_shared<WindowsDesktopEventsMonitor>();
#elif defined Q_OS_MAC
    return std::make_shared<MacDesktopEventsMonitor>();
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
