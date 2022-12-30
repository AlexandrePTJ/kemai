#pragma once

#include <QTimer>

#include "desktopEventsMonitor.h"

namespace kemai {

class WindowsDesktopEventsMonitor : public DesktopEventsMonitor
{
public:
    WindowsDesktopEventsMonitor();
    ~WindowsDesktopEventsMonitor() override = default;

    void initialize(const Settings::Events& eventsSettings) override;

private:
    void onPollTimeout();

    QTimer mPollTimer;
};

} // namespace kemai
