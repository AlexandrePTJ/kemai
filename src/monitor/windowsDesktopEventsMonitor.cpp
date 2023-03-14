#include "windowsDesktopEventsMonitor.h"

#include <Windows.h>
#include <wtsapi32.h>

#include <QApplication>

using namespace kemai;

WindowsDesktopEventsMonitor::WindowsDesktopEventsMonitor(QWidget* widget) : DesktopEventsMonitor(true, true), mListenerWidget(widget)
{
    connect(&mPollTimer, &QTimer::timeout, this, &WindowsDesktopEventsMonitor::onPollTimeout);

    qApp->installNativeEventFilter(this);
}

void WindowsDesktopEventsMonitor::initialize(const Settings::Events& eventsSettings)
{
    stop();

    mEventsSettings = eventsSettings;
}

void WindowsDesktopEventsMonitor::start()
{
    if (mEventsSettings.stopOnIdle)
    {
        mPollTimer.start(std::chrono::seconds(1));
    }

    if (mListenerWidget != nullptr)
    {
        if (mEventsSettings.stopOnLock)
        {
            WTSRegisterSessionNotification((HWND)mListenerWidget->winId(), NOTIFY_FOR_THIS_SESSION);
        }
        else
        {
            WTSUnRegisterSessionNotification((HWND)mListenerWidget->winId());
        }
    }
}

void WindowsDesktopEventsMonitor::stop()
{
    mPollTimer.stop();
    if (mListenerWidget != nullptr)
    {
        WTSUnRegisterSessionNotification((HWND)mListenerWidget->winId());
    }
}
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
bool WindowsDesktopEventsMonitor::nativeEventFilter(const QByteArray& eventType, void* message, long* /*result*/)
#else
bool WindowsDesktopEventsMonitor::nativeEventFilter(const QByteArray& eventType, void* message, qintptr* /*result*/)
#endif //
{
    if (eventType != "windows_generic_MSG" || !mEventsSettings.stopOnLock)
    {
        return false;
    }

    MSG* msg = static_cast<MSG*>(message);
    switch (msg->message)
    {
    case WM_WTSSESSION_CHANGE:
        if (msg->wParam == WTS_SESSION_LOCK || msg->wParam == WTS_SESSION_LOGOFF)
        {
            emit lockDetected();
        }
        break;

    default:
        break;
    }
    return false;
}

void WindowsDesktopEventsMonitor::onPollTimeout()
{
    LASTINPUTINFO lastInputInfo;
    lastInputInfo.cbSize = sizeof(LASTINPUTINFO);

    GetLastInputInfo(&lastInputInfo);
    auto idleSinceNMilliSecs = std::chrono::milliseconds(GetTickCount() - lastInputInfo.dwTime);

    if (mEventsSettings.stopOnIdle && mEventsSettings.idleDelayMinutes <= std::chrono::duration_cast<std::chrono::minutes>(idleSinceNMilliSecs).count())
    {
        emit idleDetected();
    }
}
