// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
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
                onStartRequested: (activityId, projectId) => root.sessionContext.startTimeSheet(activityId, projectId)
                onEditRequested: (timesheetId, editEnd, beginAt, endAt) => {
                    editDialog.timesheetId = timesheetId
                    editDialog.editEnd = editEnd
                    editDialog.title = editEnd ? "Edit End Time" : "Edit Start Time"
                    editDialog.initialDateTime = editEnd ? endAt : beginAt
                    editDialog.open()
                }
            }
        }
    }

    DateTimeEditDialog {
        id: editDialog

        property int timesheetId: 0
        property bool editEnd: false

        // Invalid dates mean "leave unchanged" on the session context side
        onConfirmed: (dateTime) => {
            if (editEnd)
                root.sessionContext.updateTimeSheetTimes(timesheetId, new Date(NaN), dateTime)
            else
                root.sessionContext.updateTimeSheetTimes(timesheetId, dateTime, new Date(NaN))
        }
    }
}
