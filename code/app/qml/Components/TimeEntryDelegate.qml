// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kemai

Rectangle {
    id: root

    required property int index
    required property string status
    required property string activityColor
    required property string projectName
    required property string activityName
    required property string description
    required property string duration
    required property string timeRange
    required property int activityId
    required property int projectId
    required property int timesheetId
    required property date beginAt
    required property date endAt

    property bool isRunning: status === "active"

    signal startRequested(int activityId, int projectId)
    signal editRequested(int timesheetId, bool editEnd, date beginAt, date endAt)

    width: ListView.view ? ListView.view.width : 0
    implicitHeight: mainLayout.implicitHeight + 22
    color: {
        if (isRunning)
            return Theme.colorAccentGreenTint
        if (hoverArea.containsMouse)
            return Qt.rgba(1, 1, 1, 0.025)
        return "transparent"
    }

    // Bottom border
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 1
        color: Theme.colorBorder
    }

    // Left color strip
    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 2
        color: root.activityColor
    }

    MouseArea {
        id: hoverArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.NoButton
    }

    RowLayout {
        id: mainLayout
        anchors {
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
            leftMargin: 16
            rightMargin: 18
        }
        spacing: 12

        // Entry info (name + description)
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2

            RowLayout {
                spacing: 7

                Text {
                    text: root.projectName
                    color: Theme.colorTextPrimary
                    font.pixelSize: Theme.fontSizeMedium
                    font.bold: true
                    elide: Text.ElideRight
                    Layout.maximumWidth: 300
                }

                Text {
                    text: "\u2014"
                    color: Theme.colorTextSecondary
                    font.pixelSize: Theme.fontSizeSmall
                }

                Text {
                    text: root.activityName
                    color: Theme.colorTextSecondary
                    font.pixelSize: Theme.fontSizeSmall
                }
            }

            Text {
                text: root.description
                color: Theme.colorTextSecondary
                font.pixelSize: Theme.fontSizeXSmall
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
        }

        // Duration + time range
        ColumnLayout {
            spacing: 2
            Layout.alignment: Qt.AlignVCenter

            Text {
                text: root.duration
                color: Theme.colorAccentGreen
                font.family: Theme.fontFamilyMono
                font.pixelSize: Theme.fontSizeMedium
                font.bold: true
                Layout.alignment: Qt.AlignRight

                SequentialAnimation on opacity {
                    running: root.isRunning
                    loops: Animation.Infinite
                    NumberAnimation { to: 0.7; duration: 500; easing.type: Easing.InOutQuad }
                    NumberAnimation { to: 1.0; duration: 500; easing.type: Easing.InOutQuad }
                }
            }

            Text {
                text: root.timeRange
                color: Theme.colorTextSecondary
                font.family: Theme.fontFamilyMono
                font.pixelSize: Theme.fontSizeXSmall
                Layout.alignment: Qt.AlignRight
            }
        }

        // Action buttons
        RowLayout {
            spacing: 5
            Layout.alignment: Qt.AlignVCenter
            Layout.minimumWidth: 120

            // Edit button (always visible)
             SecondaryButton {
                id: editButton
                text: "\u270E Edit"
                implicitHeight: 40
                implicitWidth: 50
                onClicked: editMenu.popup(editButton, 0, editButton.height)

                Menu {
                    id: editMenu

                    background: Rectangle {
                        implicitWidth: 150
                        color: Theme.colorSurfaceDark
                        border.color: Theme.colorBorder
                        border.width: 1
                        radius: Theme.radiusSmall
                    }

                    delegate: MenuItem {
                        id: menuItem
                        implicitHeight: 32

                        contentItem: Text {
                            text: menuItem.text
                            color: menuItem.enabled ? Theme.colorTextPrimary : Theme.colorTextSecondary
                            font.pixelSize: Theme.fontSizeSmall
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: 4
                        }

                        background: Rectangle {
                            color: menuItem.highlighted ? Theme.colorSurface : "transparent"
                            radius: Theme.radiusSmall
                        }
                    }

                    Action {
                        text: "Edit start time"
                        onTriggered: root.editRequested(root.timesheetId, false, root.beginAt, root.endAt)
                    }

                    Action {
                        text: "Edit end time"
                        enabled: !root.isRunning
                        onTriggered: root.editRequested(root.timesheetId, true, root.beginAt, root.endAt)
                    }
                }
            }

            // Play button (non-running entries only)
            StartStopButton {
                visible: !root.isRunning
                implicitHeight: 40
                implicitWidth: 50
                textSize: Theme.fontSizeXSmall
                onClicked: root.startRequested(root.activityId, root.projectId)
            }
        }
    }
}
