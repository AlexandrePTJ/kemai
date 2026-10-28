// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
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
    required property var tags
    required property var customValues

    property bool isRunning: status === "active"

    signal startRequested(int activityId, int projectId)
    signal editRequested

    width: ListView.view ? ListView.view.width : 0
    implicitHeight: mainLayout.implicitHeight + 22
    color: {
        if (isRunning)
            return Theme.colorAccentGreenTint
        if (rowHover.hovered)
            return Theme.colorRowHover
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

    HoverHandler {
        id: rowHover
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
                    text: "—"
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
                font.features: Theme.fontFeaturesTabular
                font.pixelSize: Theme.fontSizeMedium
                font.bold: true
                Layout.alignment: Qt.AlignRight

                SequentialAnimation on opacity {
                    running: root.isRunning
                    loops: Animation.Infinite
                    NumberAnimation {
                        to: 0.7
                        duration: 500
                        easing.type: Easing.InOutQuad
                    }
                    NumberAnimation {
                        to: 1.0
                        duration: 500
                        easing.type: Easing.InOutQuad
                    }
                }
            }

            Text {
                text: root.timeRange
                color: Theme.colorTextSecondary
                font.features: Theme.fontFeaturesTabular
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
                text: "Edit"
                iconName: "pencil"
                implicitHeight: 40
                implicitWidth: 50
                // Same visual height as the neighbouring StartStopButton (Material insets)
                topInset: 6
                bottomInset: 6
                onClicked: root.editRequested()
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
