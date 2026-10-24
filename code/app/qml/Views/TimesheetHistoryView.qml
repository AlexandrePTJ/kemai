// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import Kemai

Item {
    id: root

    required property SessionContext sessionContext

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
        }

        ListView {
            id: entriesList
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

    TimeEntryEditDialog {
        id: editDialog
        sessionContext: root.sessionContext
    }
}
