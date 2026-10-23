// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Controls

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

    contentItem: Text {
        id: buttonText
        text: root.text
        color: root.hovered ? Theme.colorTextPrimary : Theme.colorTextSecondary
        font.pixelSize: Theme.fontSizeXSmall
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        color: root.down ? Theme.colorBorder : Theme.colorSurfaceLight
        border.color: root.hovered ? Theme.colorBorderHighlight : Theme.colorBorder
        border.width: 1
        radius: Theme.radiusSmall
    }
}
