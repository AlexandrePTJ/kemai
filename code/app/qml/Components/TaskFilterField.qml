// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Layouts

import Kemai

// "Filter tasks" field of the tasks header: search icon, clear × once there is text
Rectangle {
    id: root

    property alias text: filterInput.text

    implicitWidth: 220
    implicitHeight: 30
    radius: Theme.radiusMedium
    color: Theme.colorSurface
    border.width: 1
    border.color: filterInput.activeFocus ? Theme.colorAccentGreenDim : "transparent"

    Rectangle {
        visible: filterInput.activeFocus
        anchors.fill: parent
        anchors.margins: -3
        radius: Theme.radiusMedium + 3
        color: "transparent"
        border.width: 2
        border.color: Theme.colorAccentGreenSoft
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 9
        anchors.rightMargin: 8
        spacing: 7

        Icon {
            name: "search"
            size: 13
            color: Theme.colorTextSecondary
        }

        TextInput {
            id: filterInput
            Layout.fillWidth: true
            color: Theme.colorTextPrimary
            selectionColor: Theme.colorAccentGreenDim
            font.pixelSize: Theme.fontSizeSmall
            clip: true
            Keys.onEscapePressed: filterInput.text = ""

            Text {
                visible: filterInput.text === ""
                anchors.verticalCenter: parent.verticalCenter
                text: "Filter tasks"
                color: Theme.colorTextDim
                font.pixelSize: Theme.fontSizeSmall
            }
        }

        Icon {
            visible: filterInput.text !== ""
            name: "x"
            size: 12
            color: Theme.colorTextSecondary

            TapHandler {

                gesturePolicy: TapHandler.ReleaseWithinBounds
                onTapped: filterInput.text = ""
            }

            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }
        }
    }
}
