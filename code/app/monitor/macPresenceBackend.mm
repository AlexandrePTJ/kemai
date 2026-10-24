// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "presenceMonitor.h"

#import <CoreGraphics/CoreGraphics.h>
#import <Foundation/Foundation.h>

namespace kemai
{
    /*!
     * macOS: idle time from the combined session event source; lock events from the screen lock
     * distributed notifications.
     */
    class MacPresenceBackend : public PresenceBackend
    {
    public:
        MacPresenceBackend()
        {
            auto *center = [NSDistributedNotificationCenter defaultCenter];
            m_lockObserver = [center addObserverForName:@"com.apple.screenIsLocked"
                                                 object:nil
                                                  queue:[NSOperationQueue mainQueue]
                                             usingBlock:^(NSNotification *) {
                                               if (lockChanged)
                                                   lockChanged(true);
                                             }];
            m_unlockObserver = [center addObserverForName:@"com.apple.screenIsUnlocked"
                                                   object:nil
                                                    queue:[NSOperationQueue mainQueue]
                                               usingBlock:^(NSNotification *) {
                                                 if (lockChanged)
                                                     lockChanged(false);
                                               }];
        }

        ~MacPresenceBackend() override
        {
            auto *center = [NSDistributedNotificationCenter defaultCenter];
            [center removeObserver:m_lockObserver];
            [center removeObserver:m_unlockObserver];
        }

        bool idleSupported() const override { return true; }

        std::optional<int> idleSeconds() const override
        {
            const auto seconds = CGEventSourceSecondsSinceLastEventType(kCGEventSourceStateCombinedSessionState, kCGAnyInputEventType);
            return static_cast<int>(seconds);
        }

        bool lockSupported() const override { return true; }

    private:
        id m_lockObserver   = nil;
        id m_unlockObserver = nil;
    };

    std::unique_ptr<PresenceBackend> PresenceBackend::create()
    {
        return std::make_unique<MacPresenceBackend>();
    }
} // namespace kemai
