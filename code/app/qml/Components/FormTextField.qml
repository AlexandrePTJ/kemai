// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Controls

// 36 px text field of the creation forms: accent ring on focus, red border on error
TextField {
    id: root

    property bool hasError: false
    // Shown inside the empty field (Material's own placeholder floats above it)
    property string hint: ""

    implicitHeight: 36
    leftPadding: 10
    rightPadding: 10
    topPadding: 0
    bottomPadding: 0
    topInset: 0
    bottomInset: 0
    verticalAlignment: TextInput.AlignVCenter
    color: Theme.colorTextPrimary
    selectionColor: Theme.colorAccentGreenDim
    font.pixelSize: Theme.fontSizeMedium

    Text {
        visible: root.text === "" && root.hint !== ""
        anchors.left: parent.left
        anchors.leftMargin: root.leftPadding
        anchors.verticalCenter: parent.verticalCenter
        text: root.hint
        color: Theme.colorTextDim
        font.pixelSize: Theme.fontSizeMedium
    }

    background: Rectangle {
        radius: Theme.radiusMedium
        color: Theme.colorSurface
        border.width: 1
        border.color: root.hasError ? Theme.colorAccentRed : root.activeFocus ? Theme.colorAccentGreenDim : "transparent"

        Rectangle {
            visible: root.hasError || root.activeFocus
            anchors.fill: parent
            anchors.margins: -3
            radius: Theme.radiusMedium + 3
            color: "transparent"
            border.width: 2
            border.color: root.hasError ? Theme.colorAccentRedSoft : Theme.colorAccentGreenSoft
        }
    }
}
