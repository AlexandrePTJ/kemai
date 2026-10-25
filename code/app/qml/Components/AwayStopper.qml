// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick

import Kemai

// Stops the running timer when the user is away (idle for the chosen time, or session locked, per the
// preferences), ending it when they left; tells them once they are back.
Item {
    id: root

    required property SessionContext sessionContext
    required property AppPreferences preferences
    required property PresenceMonitor presence

    // Once the user is back: kind and detail for the notification banner
    signal backAfterStop(int kind, string detail)

    // A stop is in flight: no second one
    property bool _stopping: false
    // Stop done while away, told when the user is back
    property var _pending: null
    property int _lastIdleSeconds: 0

    function _stop(end, reason) {
        root._stopping = true
        root.sessionContext.stopActiveTimeSheetAt(end, reason)
    }

    function _tellIfBack() {
        if (root._pending === null)
            return
        root.backAfterStop(root._pending.kind, root._pending.detail)
        root._pending = null
    }

    // Idle time is read only while it can stop something
    Timer {
        interval: 10000
        repeat: true
        running: root.preferences.stopOnIdle && root.presence.idleSupported && root.sessionContext.hasActiveTimesheet && !root._stopping
        onTriggered: {
            const idle = root.presence.idleSeconds()
            if (idle >= root.preferences.idleMinutes * 60)
                root._stop(new Date(Date.now() - idle * 1000), SessionController.Idle)
        }
    }

    Connections {
        target: root.presence
        function onLockedChanged() {
            if (root.presence.locked) {
                if (root.preferences.stopOnLock && root.sessionContext.hasActiveTimesheet && !root._stopping)
                    root._stop(root.presence.lockChangedAt, SessionController.Locked)
            } else {
                root._tellIfBack()
            }
        }
    }

    Connections {
        target: root.sessionContext
        function onTimeSheetStoppedAway(reason, end) {
            root._stopping = false
            const time = Qt.formatTime(end, "HH:mm")
            root._pending = reason === SessionController.Locked ? {
                kind: SessionController.TimerStoppedLocked,
                detail: time
            } : {
                kind: SessionController.TimerStoppedIdle,
                detail: time + "|" + root.preferences.idleMinutes
            }
            root._lastIdleSeconds = root.presence.idleSupported ? root.presence.idleSeconds() : 0
            // A lock stop is told at unlock; an idle stop as soon as input resumes
            if (reason === SessionController.Locked && !root.presence.locked)
                root._tellIfBack()
        }
        function onNotificationRequested() {
            // A failed stop goes through the banner as any error; allow a new attempt
            root._stopping = false
        }
    }

    // Input resumed after an idle stop: idle time drops
    Timer {
        interval: 2000
        repeat: true
        running: root._pending !== null && root._pending.kind === SessionController.TimerStoppedIdle && root.presence.idleSupported
        onTriggered: {
            const idle = root.presence.idleSeconds()
            if (idle >= 0 && idle < root._lastIdleSeconds)
                root._tellIfBack()
            else
                root._lastIdleSeconds = idle
        }
    }
}
