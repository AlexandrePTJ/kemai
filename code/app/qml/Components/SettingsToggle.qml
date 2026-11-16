// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Controls

import Kemai

// On/off switch (settings dialog, log journal: any size, the knob follows); Space toggles it when focused
AbstractButton {
    id: root

    checkable: true
    focusPolicy: Qt.StrongFocus
    implicitWidth: 34
    implicitHeight: 20
    padding: 0
    opacity: root.enabled ? 1 : 0.4

    HoverHandler {
        cursorShape: root.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
    }

    background: Rectangle {
        radius: height / 2
        color: root.checked ? Theme.colorAccentGreen : Theme.colorSurface
        border.width: root.checked ? 0 : 1
        border.color: Theme.colorBorderSubtle

        // Focus ring
        Rectangle {
            anchors.fill: parent
            anchors.margins: -3
            radius: height / 2
            color: "transparent"
            border.width: 2
            border.color: Theme.colorAccentGreenSoft
            visible: root.visualFocus
        }

        Rectangle {
            x: root.checked ? root.width - root.height + 2 : 2
            y: 3
            width: root.height - 6
            height: root.height - 6
            radius: height / 2
            color: root.checked ? Theme.colorTextOnAccent : Theme.colorTextSecondary
        }
    }

    contentItem: Item {}
}
