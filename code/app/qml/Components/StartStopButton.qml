// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kemai

Button {
    id: root

    leftPadding: 10
    rightPadding: 10

    opacity: enabled ? 1.0 : 0.5

    property alias isStarted: root.checked
    property alias textSize: buttonText.font.pixelSize

    contentItem: Item {
        implicitWidth: buttonRow.implicitWidth
        implicitHeight: buttonRow.implicitHeight

        RowLayout {
            id: buttonRow
            anchors.centerIn: parent
            spacing: 5

            Icon {
                name: root.isStarted ? "square" : "play"
                size: buttonText.font.pixelSize
                color: buttonText.color
            }

            Text {
                id: buttonText
                text: root.isStarted ? "Stop" : "Start"
                color: root.isStarted ? Theme.colorTextPrimary : Theme.colorTextDarkPrimary
                font.pixelSize: Theme.fontSizeMedium
            }
        }
    }

    background: Rectangle {
        color: {
            if (root.isStarted) {
                if (root.down) {
                    return Theme.colorButtonStopDown
                }
                return root.hovered ? Theme.colorAccentRedHover : Theme.colorButtonStop
            } else {
                if (root.down) {
                    return Theme.colorButtonStartDown
                }
                return root.hovered ? Theme.colorAccentGreenHover : Theme.colorButtonStart
            }
        }
        radius: Theme.radiusSmall
    }
}
