#pragma once

#include "desktopEventsMonitor.h"

namespace kemai {

class LinuxDesktopEventsMonitor : public DesktopEventsMonitor
{
public:
    LinuxDesktopEventsMonitor();
    ~LinuxDesktopEventsMonitor() override = default;

    void initialize(const Settings::Events& eventsSettings) override;
    void start() override;
    void stop() override;
};

} // namespace kemai
