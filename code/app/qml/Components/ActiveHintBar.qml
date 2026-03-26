// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    property bool isActive: false
    property string entryName

    implicitHeight: 30
    color: Theme.colorSurfaceDark

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 18
        anchors.rightMargin: 18
        spacing: 7

        // Pulsing green dot
        Rectangle {
            id: dot
            width: 7
            height: 7
            radius: 3.5
            color: isActive ? Theme.colorAccentGreen : Theme.colorAccentGreenTint

            SequentialAnimation on opacity {
                running: root.isActive
                loops: Animation.Infinite
                NumberAnimation { to: 0.4; duration: 1000; easing.type: Easing.InOutQuad }
                NumberAnimation { to: 1.0; duration: 1000; easing.type: Easing.InOutQuad }
            }
        }

        Text {
            text: root.isActive ? "Timer running on:" : "Timer paused"
            color: Theme.colorTextSecondary
            font.pixelSize: Theme.fontSizeXSmall
        }

        Text {
            text: root.isActive ? root.entryName : ""
            color: Theme.colorAccentGreen
            font.pixelSize: Theme.fontSizeXSmall
            font.bold: true
            elide: Text.ElideRight
            Layout.fillWidth: true
        }
    }
}
