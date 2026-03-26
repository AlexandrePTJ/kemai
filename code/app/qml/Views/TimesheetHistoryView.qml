// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Layouts

Item {
    id: root

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        EntriesHeader {
            Layout.fillWidth: true
            Layout.leftMargin: 18
            Layout.rightMargin: 18
            todayDuration: "8h 45m"
            weekDuration: "42h 15m"
            dateLabel: "Mon, Sep 15"
        }

        ListView {
            id: entriesList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            model: ListModel {
                id: timeEntries

                ListElement {
                    status: "active"
                    activityColor: "#4CAF50"
                    projectName: "Kemai Refactoring"
                    activityName: "Development"
                    description: "QML interface implementation and modern architecture"
                    duration: "02:34:15"
                    timeRange: "14:25 \u2013 now"
                }
                ListElement {
                    status: "stopped"
                    activityColor: "#f5a623"
                    projectName: "Mobile App Development"
                    activityName: "Testing"
                    description: "Bug fixes and performance optimization"
                    duration: "01:45:30"
                    timeRange: "12:15 \u2013 14:00"
                }
                ListElement {
                    status: "stopped"
                    activityColor: "#7b61ff"
                    projectName: "Website Redesign"
                    activityName: "Design"
                    description: "Creating wireframes and mockups for client review"
                    duration: "03:20:45"
                    timeRange: "08:30 \u2013 11:50"
                }
                ListElement {
                    status: "stopped"
                    activityColor: "#4ecdc4"
                    projectName: "Documentation"
                    activityName: "Writing"
                    description: "API documentation updates and user guide improvements"
                    duration: "00:55:12"
                    timeRange: "07:30 \u2013 08:25"
                }
                ListElement {
                    status: "stopped"
                    activityColor: "#e07b54"
                    projectName: "Server Maintenance"
                    activityName: "DevOps"
                    description: "Server updates and security patches deployment"
                    duration: "02:10:33"
                    timeRange: "Yesterday 16:00 \u2013 18:10"
                }
            }

            delegate: TimeEntryDelegate {}
        }
    }
}
