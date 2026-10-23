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
                onEditRequested: (timesheetId, editEnd, beginAt, endAt) => {
                    editDialog.timesheetId = timesheetId
                    editDialog.projectId = entryDelegate.projectId
                    editDialog.activityId = entryDelegate.activityId
                    editDialog.entryDescription = entryDelegate.description
                    editDialog.tags = entryDelegate.tags
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
        // Unchanged fields of the entry, sent back as is
        property int projectId: 0
        property int activityId: 0
        property string entryDescription: ""
        property var tags: []

        // Invalid dates mean "leave unchanged" on the session context side
        onConfirmed: (dateTime) => {
            if (editEnd)
                root.sessionContext.updateTimeSheet(timesheetId, projectId, activityId, entryDescription, tags, new Date(NaN), dateTime)
            else
                root.sessionContext.updateTimeSheet(timesheetId, projectId, activityId, entryDescription, tags, dateTime, new Date(NaN))
        }
    }
}
