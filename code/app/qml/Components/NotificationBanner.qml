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
    color: _level === SessionController.Error ? Theme.colorAccentRedTint : Theme.colorSurfaceDark

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
        case SessionController.CacheLoadFailed:
            return qsTr("Failed to load projects and activities.")
        case SessionController.CacheRefreshFailed:
            return qsTr("Could not refresh projects and activities — showing the previous list.")
        case SessionController.TimerStoppedIdle: {
            // detail: "HH:mm|minutes"
            const parts = detail.split("|")
            return qsTr("Timer stopped at %1 after %2 minutes of inactivity.").arg(parts[0]).arg(parts[1])
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
        anchors.left: parent.left
        anchors.leftMargin: Theme.spacingLarge
        anchors.right: closeButton.left
        anchors.verticalCenter: parent.verticalCenter
        color: root._level === SessionController.Error ? Theme.colorAccentRed : Theme.colorTextPrimary
        font.pixelSize: Theme.fontSizeSmall
        elide: Text.ElideRight
    }

    Text {
        id: closeButton
        anchors.right: parent.right
        anchors.rightMargin: Theme.spacingLarge
        anchors.verticalCenter: parent.verticalCenter
        text: "✕"
        color: Theme.colorTextSecondary
        font.pixelSize: Theme.fontSizeSmall

        MouseArea {
            anchors.fill: parent
            anchors.margins: -8
            cursorShape: Qt.PointingHandCursor
            onClicked: root.visible = false
        }
    }
}
