import Kemai
// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: root

    required property SessionContext sessionContext
    required property AppPreferences preferences
    required property PresenceMonitor presence
    required property UpdateNotifier updates
    // At least one settings section exists: the account menu offers the settings
    property bool settingsAvailable: false
    // Back to the foreground: reload what may have changed on the server meanwhile
    readonly property bool windowActive: Window.active

    signal logoutRequested()
    signal settingsRequested()
    signal logJournalRequested()
    signal aboutRequested()

    onWindowActiveChanged: {
        if (windowActive) {
            root.sessionContext.refreshIfStale();
            root.sessionContext.syncActiveTimeSheet();
        }
    }

    Connections {
        function onNotificationRequested(level, kind, detail) {
            notificationBanner.show(level, kind, detail);
        }

        target: root.sessionContext
    }

    ResourceCreationDialog {
        id: timerBarCreation

        objectName: "timerBarCreation"
        sessionContext: root.sessionContext
        onPairChosen: (pair) => {
            return timerBar.selectPair(pair);
        }
        onCreated: (detail) => {
            return notificationBanner.show(SessionController.Success, SessionController.ResourceCreated, detail);
        }
    }

    AwayStopper {
        sessionContext: root.sessionContext
        preferences: root.preferences
        presence: root.presence
        onBackAfterStop: (kind, detail) => {
            return notificationBanner.show(SessionController.Info, kind, detail);
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
            id: timerBar

            Layout.fillWidth: true
            timerText: root.sessionContext.activeTimesheetDuration
            entryDescription: root.sessionContext.activeTimesheetLabel
            isRunning: root.sessionContext.hasActiveTimesheet
            activityModel: root.sessionContext.activityModel
            onStartClicked: (activityId, projectId) => {
                return historyView.startEntry(activityId, projectId, null);
            }
            onStopClicked: root.sessionContext.stopActiveTimeSheet()
            onCreateRequested: (kind, name) => {
                return timerBarCreation.openFor(kind, name, root.sessionContext.lastUsedProjectId());
            }
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
            taskTitle: root.sessionContext.hasTaskManagement ? root.sessionContext.tasks.runningTaskTitle : ""
            since: root.sessionContext.activeTimesheetBegin

            ViewSwitch {
                visible: root.sessionContext.hasTaskManagement
                compact: root.width < 640
                current: historyView.tasksShown ? "tasks" : "history"
                taskCount: root.sessionContext.tasks.totalCount
                taskCountVisible: root.sessionContext.tasksLoad === SessionController.Ready
                onSelected: (view) => {
                    return historyView.view = view;
                }
            }

        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: Theme.colorBorder
        }

        // A newer Kemai, or the result of a check started from the account menu
        UpdateStrip {
            Layout.fillWidth: true
            notifier: root.updates
        }

        TimesheetHistoryView {
            id: historyView

            Layout.fillWidth: true
            Layout.fillHeight: true
            sessionContext: root.sessionContext
            preferences: root.preferences
            updates: root.updates
            settingsAvailable: root.settingsAvailable
            onLogoutRequested: root.logoutRequested()
            onSettingsRequested: root.settingsRequested()
            onLogJournalRequested: root.logJournalRequested()
            onAboutRequested: root.aboutRequested()
            onResourceCreated: (detail) => {
                return notificationBanner.show(SessionController.Success, SessionController.ResourceCreated, detail);
            }
        }

    }

    background: Rectangle {
        color: Theme.colorBackground
    }

}
