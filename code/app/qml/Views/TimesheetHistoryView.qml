// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kemai

Item {
    id: root

    required property SessionContext sessionContext
    required property UpdateNotifier updates

    // The user chose to log out (and confirmed it if a timer runs)
    property bool settingsAvailable: false
    // History or tasks (Task Management plugin only); kept for the session's life
    property string _view: "history"
    readonly property bool _tasksShown: root.sessionContext.hasTaskManagement && root._view === "tasks"

    signal logoutRequested
    signal settingsRequested
    signal logJournalRequested
    // A resource was created from the edit dialog: NotificationKind.ResourceCreated detail
    signal resourceCreated(string detail)

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        EntriesHeader {
            Layout.fillWidth: true
            Layout.leftMargin: 18
            Layout.rightMargin: 18
            todayDuration: root.sessionContext.recentTimeSheets.todayDuration
            weekDuration: root.sessionContext.recentTimeSheets.weekDuration
            dateLabel: root.sessionContext.recentTimeSheets.dateLabel
            refreshing: root.sessionContext.refreshing
            onRefreshClicked: root.sessionContext.refresh()
            titleVisible: !root.sessionContext.hasTaskManagement
            statsVisible: !root._tasksShown

            leading: ViewSwitch {
                current: root._tasksShown ? "tasks" : "history"
                taskCount: root.sessionContext.tasks.totalCount
                taskCountVisible: root.sessionContext.tasksLoad === SessionController.Ready
                onSelected: view => root._view = view
            }

            middle: [
                TaskFilterField {
                    visible: root._tasksShown
                    text: root.sessionContext.tasks.filterText
                    onTextChanged: root.sessionContext.tasks.filterText = text
                },
                AbstractButton {
                    id: tasksRefresh
                    visible: root._tasksShown
                    Layout.preferredWidth: 28
                    Layout.preferredHeight: 28
                    onClicked: root.sessionContext.loadTasks()

                    ToolTip.visible: tasksRefresh.hovered
                    ToolTip.delay: 500
                    ToolTip.text: "Reload tasks"

                    HoverHandler {
                        cursorShape: Qt.PointingHandCursor
                    }

                    background: Rectangle {
                        radius: Theme.radiusMedium
                        color: tasksRefresh.hovered ? Theme.colorSurface : "transparent"
                        border.width: 1
                        border.color: Theme.colorBorderSubtle
                    }

                    contentItem: Item {
                        Icon {
                            anchors.centerIn: parent
                            name: "refresh-cw"
                            size: 13
                            color: Theme.colorTextSecondary
                        }
                    }
                }
            ]

            AccountMenu {
                accountName: root.sessionContext.accountName
                host: root.sessionContext.host
                timerRunning: root.sessionContext.hasActiveTimesheet
                settingsAvailable: root.settingsAvailable
                updates: root.updates
                onSettingsRequested: root.settingsRequested()
                onLogJournalRequested: root.logJournalRequested()
                onLogoutRequested: root.sessionContext.hasActiveTimesheet ? logoutConfirm.open() : root.logoutRequested()
            }
        }

        TasksPane {
            visible: root._tasksShown
            Layout.fillWidth: true
            Layout.fillHeight: true
            sessionContext: root.sessionContext
            onBackToHistoryRequested: root._view = "history"
        }

        ListView {
            id: entriesList

            WheelScroller {
                flickable: entriesList
            }
            visible: !root._tasksShown
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            model: root.sessionContext.recentTimeSheets

            delegate: TimeEntryDelegate {
                id: entryDelegate

                onStartRequested: (activityId, projectId) => root.sessionContext.startTimeSheet(activityId, projectId)
                onEditRequested: editDialog.openFor(entryDelegate)
            }
        }
    }

    LogoutConfirmDialog {
        id: logoutConfirm
        sessionContext: root.sessionContext
        onConfirmed: root.logoutRequested()
    }

    TimeEntryEditDialog {
        id: editDialog
        sessionContext: root.sessionContext
        onResourceCreated: detail => root.resourceCreated(detail)
    }
}
