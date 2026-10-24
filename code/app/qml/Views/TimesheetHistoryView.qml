// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import Kemai

Item {
    id: root

    required property SessionContext sessionContext

    // The user chose to log out (and confirmed it if a timer runs)
    signal logoutRequested()

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

            AccountMenu {
                accountName: root.sessionContext.accountName
                host: root.sessionContext.host
                timerRunning: root.sessionContext.hasActiveTimesheet
                onLogoutRequested: root.sessionContext.hasActiveTimesheet ? logoutConfirm.open() : root.logoutRequested()
            }
        }

        ListView {
            id: entriesList

            WheelScroller {
                flickable: entriesList
            }
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
    }
}
