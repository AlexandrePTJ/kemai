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
    void start() override;
    void stop() override;

private:
    void onPollTimeout();

    QTimer mPollTimer;
    Settings::Events mEventsSettings;
};

} // namespace kemai
