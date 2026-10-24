// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "presenceMonitor.h"

// Windows headers
#include <windows.h>
#include <wtsapi32.h>

// 3rd party headers
#include <spdlog/spdlog.h>

namespace
{
    constexpr auto WindowClassName = L"KemaiPresenceListener";
}

namespace kemai
{
    /*!
     * Windows: idle time from GetLastInputInfo; lock events from WTS session notifications, received by a
     * message-only window of our own (the QML window's handle is not ours to subclass).
     */
    class WindowsPresenceBackend : public PresenceBackend
    {
    public:
        WindowsPresenceBackend()
        {
            WNDCLASSEXW windowClass{};
            windowClass.cbSize        = sizeof(windowClass);
            windowClass.lpfnWndProc   = &WindowsPresenceBackend::windowProc;
            windowClass.hInstance     = GetModuleHandleW(nullptr);
            windowClass.lpszClassName = WindowClassName;
            RegisterClassExW(&windowClass);

            m_window = CreateWindowExW(0, WindowClassName, L"", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, windowClass.hInstance, nullptr);
            if (m_window == nullptr)
            {
                spdlog::warn("No window for session lock events: error {}", GetLastError());
                return;
            }
            SetWindowLongPtrW(m_window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
            m_lockSupported = WTSRegisterSessionNotification(m_window, NOTIFY_FOR_THIS_SESSION) != FALSE;
        }

        ~WindowsPresenceBackend() override
        {
            if (m_window != nullptr)
            {
                WTSUnRegisterSessionNotification(m_window);
                DestroyWindow(m_window);
            }
        }

        bool idleSupported() const override { return true; }

        std::optional<int> idleSeconds() const override
        {
            LASTINPUTINFO info{};
            info.cbSize = sizeof(info);
            if (GetLastInputInfo(&info) == FALSE)
            {
                return std::nullopt;
            }
            // Both tick counts wrap after 49.7 days: the unsigned difference stays right
            return static_cast<int>((GetTickCount() - info.dwTime) / 1000);
        }

        bool lockSupported() const override { return m_lockSupported; }

    private:
        static LRESULT CALLBACK windowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
        {
            if (message == WM_WTSSESSION_CHANGE)
            {
                auto *self = reinterpret_cast<WindowsPresenceBackend *>(GetWindowLongPtrW(window, GWLP_USERDATA));
                if (self != nullptr && self->lockChanged)
                {
                    if (wParam == WTS_SESSION_LOCK || wParam == WTS_SESSION_LOGOFF)
                    {
                        self->lockChanged(true);
                    }
                    else if (wParam == WTS_SESSION_UNLOCK || wParam == WTS_SESSION_LOGON)
                    {
                        self->lockChanged(false);
                    }
                }
                return 0;
            }
            return DefWindowProcW(window, message, wParam, lParam);
        }

        HWND m_window        = nullptr;
        bool m_lockSupported = false;
    };

    std::unique_ptr<PresenceBackend> PresenceBackend::create()
    {
        return std::make_unique<WindowsPresenceBackend>();
    }
} // namespace kemai
