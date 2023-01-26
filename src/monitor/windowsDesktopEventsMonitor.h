#pragma once

#include <QAbstractNativeEventFilter>
#include <QTimer>

#include "desktopEventsMonitor.h"

namespace kemai {

class WindowsDesktopEventsMonitor : public DesktopEventsMonitor, public QAbstractNativeEventFilter
{
public:
    WindowsDesktopEventsMonitor(QWidget* widget);
    ~WindowsDesktopEventsMonitor() override = default;

    void initialize(const Settings::Events& eventsSettings) override;
    void start() override;
    void stop() override;

    bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) override;

private:
    void onPollTimeout();

    QWidget* mListenerWidget = nullptr;
    QTimer mPollTimer;
    Settings::Events mEventsSettings;
};

} // namespace kemai
