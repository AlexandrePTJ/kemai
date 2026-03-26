// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Controls

Button {
    id: root

    leftPadding: 10
    rightPadding: 10

    property alias isStarted: root.checked
    property alias textSize: buttonText.font.pixelSize

    contentItem: Text {
        id: buttonText
        text: isStarted ? "\u25A0 Stop" : "\u25B6 Start"
        color: isStarted ? Theme.colorTextPrimary : Theme.colorTextDarkPrimary
        font.pixelSize: Theme.fontSizeMedium
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        color: {
            if (root.isStarted) {
                if (root.down) {
                    return Theme.colorButtonStopDown
                }
                return root.hovered ? Theme.colorAccentRedHover : Theme.colorButtonStop
            }
            else {
                if (root.down) {
                    return Theme.colorButtonStartDown
                }
                return root.hovered ? Theme.colorAccentGreenHover : Theme.colorButtonStart
            }
        }
        radius: Theme.radiusSmall
    }
}
