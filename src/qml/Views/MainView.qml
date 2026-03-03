// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: root

    // Active timer state: 02:34:15 = 9255 seconds
    property int activeSeconds: 9255

    function formatDuration(secs) {
        var h = Math.floor(secs / 3600)
        var m = Math.floor((secs % 3600) / 60)
        var s = secs % 60
        return String(h).padStart(2, "0") + ":" +
               String(m).padStart(2, "0") + ":" +
               String(s).padStart(2, "0")
    }

    /*Timer {
        interval: 1000
        repeat: true
        running: true
        onTriggered: {
            root.activeSeconds++
            timeEntries.setProperty(0, "duration", root.formatDuration(root.activeSeconds))
        }
    }*/

    background: Rectangle {
        color: Theme.colorBackground
    }

    header: ToolBar {
        background: Rectangle {
            color: Theme.colorBackgroundDark
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 8
            anchors.rightMargin: 8

            Item { Layout.fillWidth: true }

            Text {
                text: "⏱ Kemai"
                color: Theme.colorTextPrimary
                font.pixelSize: Theme.fontSizeXLarge
                font.bold: true
                Layout.alignment: Qt.AlignHCenter
            }

            Item { Layout.fillWidth: true }

            Button {
                text: "⚙"
                flat: true
                font.pixelSize: 18
                contentItem: Text {
                    text: parent.text
                    color: Theme.colorTextPrimary
                    font: parent.font
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Item {}
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.marginsMain
        spacing: Theme.spacingMedium

        // Timesheet selection and/or current running
        CurrentTimesheetSelectionItem {
            Layout.fillWidth: true
            height: 50
        }

        // Separator
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: Theme.colorSurface
        }

        // Timeheets history
        TimesheetHistoryItem {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
