// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Layouts

Item {
    id: root

    property string todayDuration: "0h 00m"
    property string weekDuration: "0h 00m"
    property string dateLabel: ""

    implicitHeight: 38

    RowLayout {
        anchors.fill: parent
        spacing: 8

        // Title
        RowLayout {
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

        // Stats
        RowLayout {
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
    }
}
