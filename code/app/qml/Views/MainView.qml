// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: root

    required property SessionContext sessionContext

    property bool timerRunning: true
    property int activeSeconds: 9255

    function formatDuration(secs) {
        var h = Math.floor(secs / 3600)
        var m = Math.floor((secs % 3600) / 60)
        var s = secs % 60
        return String(h).padStart(2, "0") + ":" +
               String(m).padStart(2, "0") + ":" +
               String(s).padStart(2, "0")
    }

    background: Rectangle {
        color: Theme.colorBackground
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        TimerBar {
            Layout.fillWidth: true
            timerText: root.formatDuration(root.activeSeconds)
            entryDescription: "Kemai Refactoring - Development"
            isRunning: root.timerRunning
            onStopClicked: root.timerRunning = false
            onStartClicked: root.timerRunning = true
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: Theme.colorBorder
        }

        ActiveHintBar {
            Layout.fillWidth: true
            isActive: root.timerRunning
            entryName: "Kemai Refactoring - Development"
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: Theme.colorBorder
        }

        TimesheetHistoryView {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
