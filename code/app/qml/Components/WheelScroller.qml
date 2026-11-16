// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick

import Kemai

// Mouse wheel scrolling for a Flickable (declare it inside the view: `WheelScroller { flickable: myList }`).
// Qt's own wheel handling restarts its animation on every event and barely moves with high-resolution wheels
// (1/8-notch events): scroll a fixed distance per notch instead, easing toward a target that each event moves.
// Decided by the event, not the device: Qt may report a mouse wheel as a touchpad.
// Pixel deltas (touchpads, macOS trackpads with their own momentum) are applied as is.
WheelHandler {
    id: root

    required property Flickable flickable
    property real step: 120

    property real _target: 0

    acceptedDevices: PointerDevice.AllDevices

    function _clamped(y) {
        const maxY = root.flickable.originY + Math.max(0, root.flickable.contentHeight - root.flickable.height)
        return Math.max(root.flickable.originY, Math.min(maxY, y))
    }

    onWheel: event => {
        if (event.pixelDelta.y !== 0) {
            root._ease.stop()
            root.flickable.contentY = root._clamped(root.flickable.contentY - event.pixelDelta.y)
            return
        }
        const from = root._ease.running ? root._target : root.flickable.contentY
        root._target = root._clamped(from - event.angleDelta.y / 120 * root.step)
        root._ease.start()
    }

    readonly property FrameAnimation _ease: FrameAnimation {
        onTriggered: {
            const diff = root._target - root.flickable.contentY
            if (Math.abs(diff) < 1) {
                root.flickable.contentY = root._target
                root._ease.stop()
                return
            }
            root.flickable.contentY += diff * Math.min(1, root._ease.frameTime * 18)
        }
    }

    // A drag or flick takes over from the wheel
    readonly property Connections _drag: Connections {
        target: root.flickable
        function onMovementStarted() {
            root._ease.stop()
        }
    }
}
