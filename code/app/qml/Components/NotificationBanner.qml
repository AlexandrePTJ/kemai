// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick

import Kemai

// Single notification channel of the UI: composes the human-readable message
// from the NotificationKind — the only place where API errors become words.
Rectangle {
    id: root

    height: visible ? 36 : 0
    visible: false
    color: _level === SessionController.Error ? Theme.colorAccentRedTint : _level === SessionController.Success ? Theme.colorAccentGreenSoft : Theme.colorSurfaceDark

    property int _level: SessionController.Info

    function show(level, kind, detail) {
        _level = level
        messageText.text = _messageForKind(kind, detail)
        visible = true
        hideTimer.restart()
    }

    function _messageForKind(kind, detail) {
        switch (kind) {
        case SessionController.NoNetwork:
            return qsTr("Network unavailable — check your connection.")
        case SessionController.Forbidden:
            return qsTr("You don't have permission for this action.")
        case SessionController.NotFound:
            return qsTr("Resource not found on the server.")
        case SessionController.ServerError:
            return qsTr("The Kimai server returned an error.")
        case SessionController.InvalidResponse:
            return qsTr("Unexpected response from the server.")
        case SessionController.HttpsRedirect:
            return qsTr("This server only answers over https: connect with %1.").arg(detail)
        case SessionController.InvalidValue:
            return detail !== "" ? qsTr("Kimai refused a value: %1").arg(detail.replace(/&/g, "&amp;").replace(/</g, "&lt;")) : qsTr("Kimai refused a value.")
        case SessionController.CacheLoadFailed:
            return qsTr("Failed to load projects and activities.")
        case SessionController.CacheRefreshFailed:
            return qsTr("Could not refresh projects and activities — showing the previous list.")
        case SessionController.TimerStoppedIdle:
            {
                // detail: "HH:mm|minutes"
                const parts = detail.split("|")
                return Number(parts[1]) === 1 ? qsTr("Timer stopped at %1 after 1 minute of inactivity.").arg(parts[0]) : qsTr("Timer stopped at %1 after %2 minutes of inactivity.").arg(parts[0]).arg(parts[1])
            }
        case SessionController.ResourceCreated:
            {
                // detail: "activity|name|project" or "project|name|"
                const parts = detail.split("|")
                const name = "<b>" + parts[1].replace(/&/g, "&amp;").replace(/</g, "&lt;") + "</b>"
                if (parts[0] === "project")
                    return qsTr("Project %1 created").arg(name)
                return parts[2] !== "" ? qsTr("Activity %1 created in %2").arg(name).arg(parts[2].replace(/&/g, "&amp;").replace(/</g, "&lt;")) : qsTr("Global activity %1 created").arg(name)
            }
        case SessionController.TimerStoppedLocked:
            return qsTr("Timer stopped at %1, when the session was locked.").arg(detail)
        default:
            return qsTr("An unexpected error occurred.")
        }
    }

    Timer {
        id: hideTimer
        interval: 6000
        onTriggered: root.visible = false
    }

    Text {
        id: messageText
        anchors.left: successIcon.visible ? successIcon.right : parent.left
        anchors.leftMargin: successIcon.visible ? 10 : Theme.spacingLarge
        // Messages may hold <b>; data inside them is escaped
        textFormat: Text.StyledText
        anchors.right: closeButton.left
        anchors.verticalCenter: parent.verticalCenter
        color: root._level === SessionController.Error ? Theme.colorAccentRed : Theme.colorTextPrimary
        font.pixelSize: Theme.fontSizeSmall
        elide: Text.ElideRight
    }

    Icon {
        id: successIcon
        visible: root._level === SessionController.Success
        anchors.left: parent.left
        anchors.leftMargin: Theme.spacingLarge
        anchors.verticalCenter: parent.verticalCenter
        name: "check"
        size: 14
        color: Theme.colorAccentGreen
    }

    Rectangle {
        visible: root._level === SessionController.Success
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 1
        color: Theme.colorAccentGreenDim
    }

    Text {
        id: closeButton
        anchors.right: parent.right
        anchors.rightMargin: Theme.spacingLarge
        anchors.verticalCenter: parent.verticalCenter
        text: "✕"
        color: Theme.colorTextSecondary
        font.pixelSize: Theme.fontSizeSmall

        TapHandler {

            gesturePolicy: TapHandler.ReleaseWithinBounds
            margin: 8
            onTapped: root.visible = false
        }

        HoverHandler {
            margin: 8
            cursorShape: Qt.PointingHandCursor
        }
    }
}
