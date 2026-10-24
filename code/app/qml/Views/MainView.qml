// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: root

    required property SessionContext sessionContext

    // At least one settings section exists: the account menu offers the settings
    property bool settingsAvailable: false

    signal logoutRequested()
    signal settingsRequested()

    // Back to the foreground: reload what may have changed on the server meanwhile
    readonly property bool windowActive: Window.active
    onWindowActiveChanged: {
        if (windowActive) {
            root.sessionContext.refreshIfStale()
            root.sessionContext.syncActiveTimeSheet()
        }
    }

    background: Rectangle {
        color: Theme.colorBackground
    }

    Connections {
        target: root.sessionContext
        function onNotificationRequested(level, kind, detail) {
            notificationBanner.show(level, kind, detail)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        NotificationBanner {
            id: notificationBanner
            Layout.fillWidth: true
        }

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
            Layout.preferredHeight: 1
            color: Theme.colorBorder
        }

        ActiveHintBar {
            Layout.fillWidth: true
            isActive: root.sessionContext.hasActiveTimesheet
            entryName: root.sessionContext.activeTimesheetLabel
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: Theme.colorBorder
        }

        TimesheetHistoryView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            sessionContext: root.sessionContext
            settingsAvailable: root.settingsAvailable
            onLogoutRequested: root.logoutRequested()
            onSettingsRequested: root.settingsRequested()
        }
    }
}
