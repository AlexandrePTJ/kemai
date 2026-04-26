// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: root

    required property SessionContext sessionContext

    background: Rectangle {
        color: Theme.colorBackground
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        TimerBar {
            Layout.fillWidth: true
            timerText: root.sessionContext.activeTimesheetDuration
            entryDescription: root.sessionContext.activeTimesheetLabel
            isRunning: root.sessionContext.hasActiveTimesheet
            activityModel: root.sessionContext.activityModel
            onStartClicked: (activityId, projectId) => root.sessionContext.startTimeSheet(activityId, projectId)
            onStopClicked: root.sessionContext.stopActiveTimeSheet()
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: Theme.colorBorder
        }

        ActiveHintBar {
            Layout.fillWidth: true
            isActive: root.sessionContext.hasActiveTimesheet
            entryName: root.sessionContext.activeTimesheetLabel
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: Theme.colorBorder
        }

        TimesheetHistoryView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            sessionContext: root.sessionContext
        }
    }
}
