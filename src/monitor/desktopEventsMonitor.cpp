#include "desktopEventsMonitor.h"

#ifdef Q_OS_WINDOWS
#    include "windowsDesktopEventsMonitor.h"
#elif defined Q_OS_MAC
#    include "macDesktopEventsMonitor.h"
#elif defined Q_OS_LINUX
#    include "linuxDesktopEventsMonitor.h"
#endif

using namespace kemai;

DesktopEventsMonitor::DesktopEventsMonitor(bool hasLockSupport, bool hasIdleSupport) : mHasLockSupport(hasLockSupport), mHasIdleSupport(hasIdleSupport) {}

std::shared_ptr<DesktopEventsMonitor> DesktopEventsMonitor::create(QWidget* widget)
{
#ifdef Q_OS_WINDOWS
    return std::make_shared<WindowsDesktopEventsMonitor>(widget);
#elif defined Q_OS_MAC
    return std::make_shared<MacDesktopEventsMonitor>();
#elif defined Q_OS_LINUX
    return std::make_shared<LinuxDesktopEventsMonitor>();
#else
    return {};
#endif //
}

bool DesktopEventsMonitor::hasLockSupport() const
{
    return mHasLockSupport;
}

bool DesktopEventsMonitor::hasIdleSupport() const
{
    return mHasIdleSupport;
}
