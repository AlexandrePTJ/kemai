import Kemai
// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    property bool isActive: false
    property string entryName
    // The running timer belongs to this task (Task Management), started at `since`
    property string taskTitle: ""
    property date since
    // Controls at the right end (the History | Tasks switch); the text gives way to them
    default property alias trailing: trailingRow.data
    readonly property bool _hasTrailing: trailingRow.visibleChildren.length > 0

    implicitHeight: root._hasTrailing ? 34 : 30
    color: Theme.colorSurfaceDark

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: root._hasTrailing ? 16 : 18
        anchors.rightMargin: root._hasTrailing ? 12 : 18
        spacing: 7

        // Pulsing green dot
        Rectangle {
            id: dot

            Layout.preferredWidth: 7
            Layout.preferredHeight: 7
            radius: 3.5
            color: root.isActive ? Theme.colorAccentGreen : Theme.colorAccentGreenTint

            SequentialAnimation on opacity {
                running: root.isActive
                loops: Animation.Infinite

                NumberAnimation {
                    to: 0.4
                    duration: 1000
                    easing.type: Easing.InOutQuad
                }

                NumberAnimation {
                    to: 1
                    duration: 1000
                    easing.type: Easing.InOutQuad
                }

            }

        }

        Text {
            text: !root.isActive ? "Timer paused" : root.taskTitle !== "" ? "Timer running on task" : "Timer running on:"
            color: Theme.colorTextSecondary
            font.pixelSize: Theme.fontSizeXSmall
        }

        Text {
            text: !root.isActive ? "" : root.taskTitle !== "" ? "“" + root.taskTitle + "”" : root.entryName
            color: Theme.colorAccentGreen
            font.pixelSize: Theme.fontSizeXSmall
            font.bold: true
            elide: Text.ElideRight
            // Its own width at most, less when the switch needs the room
            Layout.fillWidth: true
            Layout.maximumWidth: Math.ceil(implicitWidth)
        }

        Text {
            visible: root.isActive && root.taskTitle !== "" && !isNaN(root.since.getTime())
            text: "since " + Qt.formatTime(root.since, "HH:mm")
            color: Theme.colorTextSecondary
            font.pixelSize: Theme.fontSizeXSmall
            font.features: Theme.fontFeaturesTabular
        }

        Item {
            Layout.fillWidth: true
        }

        RowLayout {
            id: trailingRow

            spacing: 8
        }

    }

}
