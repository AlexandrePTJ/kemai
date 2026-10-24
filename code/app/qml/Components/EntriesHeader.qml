// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Layouts

Item {
    id: root

    property string todayDuration: "0h 00m"
    property string weekDuration: "0h 00m"
    property string dateLabel: ""
    property bool refreshing: false

    // Extra controls at the far right, after a divider (e.g. the account menu)
    default property alias trailing: trailingRow.data
    // Replaces the title when set (e.g. the History | Tasks switch)
    property alias leading: leadingRow.data
    // Right of the spacer, in place of the stats and refresh when those are hidden (e.g. the tasks filter)
    property alias middle: middleRow.data
    property bool statsVisible: true
    // False when `leading` takes the title's place
    property bool titleVisible: true

    signal refreshClicked()

    implicitHeight: 38

    RowLayout {
        anchors.fill: parent
        spacing: 8

        RowLayout {
            id: leadingRow
            visible: !root.titleVisible
            spacing: 8
        }

        // Title
        RowLayout {
            visible: root.titleVisible
            spacing: 8

            Text {
                text: "\uD83D\uDCC5"
                font.pixelSize: 16
            }

            Text {
                text: "Time Entries"
                color: Theme.colorTextPrimary
                font.pixelSize: Theme.fontSizeMedium
                font.bold: true
            }
        }

        Item { Layout.fillWidth: true }

        RowLayout {
            id: middleRow
            spacing: 8
        }

        // Stats
        RowLayout {
            visible: root.statsVisible
            spacing: 4

            Text {
                text: "Today:"
                color: Theme.colorTextSecondary
                font.pixelSize: Theme.fontSizeSmall
            }
            Text {
                text: root.todayDuration
                color: Theme.colorAccentGreen
                font.pixelSize: Theme.fontSizeSmall
                font.bold: true
                font.family: Theme.fontFamilyMono
            }

            Text {
                text: "\u00B7"
                color: Theme.colorBorderHighlight
                font.pixelSize: Theme.fontSizeSmall
                Layout.leftMargin: 8
                Layout.rightMargin: 8
            }

            Text {
                text: "This week:"
                color: Theme.colorTextSecondary
                font.pixelSize: Theme.fontSizeSmall
            }
            Text {
                text: root.weekDuration
                color: Theme.colorAccentGreen
                font.pixelSize: Theme.fontSizeSmall
                font.bold: true
                font.family: Theme.fontFamilyMono
            }

            Text {
                text: "\u00B7"
                color: Theme.colorBorderHighlight
                font.pixelSize: Theme.fontSizeSmall
                visible: root.dateLabel.length > 0
                Layout.leftMargin: 8
                Layout.rightMargin: 8
            }

            Text {
                text: root.dateLabel
                color: Theme.colorTextSecondary
                font.pixelSize: Theme.fontSizeSmall
                visible: root.dateLabel.length > 0
            }
        }

        // Refresh (projects/activities cache + timesheets)
        Text {
            id: refreshIcon
            visible: root.statsVisible
            text: "\u21BB"
            color: root.refreshing ? Theme.colorTextSecondary : Theme.colorTextPrimary
            font.pixelSize: Theme.fontSizeMedium
            Layout.leftMargin: 8

            RotationAnimator on rotation {
                running: root.refreshing
                from: 0
                to: 360
                duration: 1000
                loops: Animation.Infinite
                onStopped: refreshIcon.rotation = 0
            }

            MouseArea {
                anchors.fill: parent
                anchors.margins: -6
                enabled: !root.refreshing
                cursorShape: Qt.PointingHandCursor
                onClicked: root.refreshClicked()
            }
        }

        Rectangle {
            visible: trailingRow.children.length > 0
            Layout.leftMargin: 6
            Layout.rightMargin: 6
            Layout.preferredWidth: 1
            Layout.preferredHeight: 20
            color: Theme.colorDivider
        }

        RowLayout {
            id: trailingRow
            spacing: 8
        }
    }
}
