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

    // Material's Button insets the background (6px top/bottom): fill the size given by the layout
    topInset: 0
    bottomInset: 0
    leftInset: 0
    rightInset: 0

    property alias textSize: buttonText.font.pixelSize
    // Lucide icon before the text, none when empty
    property string iconName: ""

    contentItem: Item {
        implicitWidth: buttonRow.implicitWidth
        implicitHeight: buttonRow.implicitHeight

        RowLayout {
            id: buttonRow
            anchors.centerIn: parent
            spacing: 4

            Icon {
                visible: root.iconName !== ""
                name: root.iconName
                size: buttonText.font.pixelSize
                color: buttonText.color
            }

            Text {
                id: buttonText
                text: root.text
                color: root.hovered ? Theme.colorTextPrimary : Theme.colorTextSecondary
                font.pixelSize: Theme.fontSizeXSmall
            }
        }
    }

    background: Rectangle {
        color: root.down ? Theme.colorBorder : Theme.colorSurfaceLight
        border.color: root.hovered ? Theme.colorBorderHighlight : Theme.colorBorder
        border.width: 1
        radius: Theme.radiusSmall
    }
}
