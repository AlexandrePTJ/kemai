#pragma once

#include <memory>

#include <QObject>
#include <QWidget>

#include "settings/settings.h"

namespace kemai {

class DesktopEventsMonitor : public QObject
{
    Q_OBJECT

protected:
    DesktopEventsMonitor(bool hasLockSupport, bool hasIdleSupport);

public:
    static std::shared_ptr<DesktopEventsMonitor> create();

    virtual void initialize(const Settings::Events& eventsSettings) = 0;
    virtual void start()                                            = 0;
    virtual void stop()                                             = 0;

    bool hasLockSupport() const;
    bool hasIdleSupport() const;

signals:
    void lockDetected();
    void idleDetected();

private:
    const bool mHasLockSupport = false;
    const bool mHasIdleSupport = false;
};

} // namespace kemai
