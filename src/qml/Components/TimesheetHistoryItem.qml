// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {

    ColumnLayout {
        anchors.fill: parent

        // Stats
        RowLayout {
            Layout.fillWidth: true    
            spacing: 12

            Text {
                text: "📊 Time Entries"
                color: Theme.colorTextPrimary
                font.pixelSize: 15
                font.bold: true
            }

            Item { Layout.fillWidth: true }

            Text {
                text: "Today: "
                color: Theme.colorTextSecondary
                font.pixelSize: 13
            }

            Text {
                text: "8h 45m"
                color: Theme.colorAccentGreen
                font.pixelSize: 13
                font.bold: true
            }

            Text {
                text: "  This week: "
                color: Theme.colorTextSecondary
                font.pixelSize: 13
            }

            Text {
                text: "42h 15m"
                color: Theme.colorAccentGreen
                font.pixelSize: 13
                font.bold: true
            }
        }

        // ── Entries List ──
        ListView {
            id: entriesList
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 4
            clip: true

            model: ListModel {
                id: timeEntries

                ListElement {
                    status: "active"
                    activityColor: "#4CAF50"
                    projectName: "Kemai Refactoring"
                    activityName: "Development"
                    description: "QML interface implementation and modern arch."
                    duration: "02:34:15"
                    timeRange: "14:25 – now"
                }
                ListElement {
                    status: "paused"
                    activityColor: "#FF9800"
                    projectName: "Mobile App Development"
                    activityName: "Testing"
                    description: "Bug fixes and performance optimization"
                    duration: "01:45:30"
                    timeRange: "12:15 – 14:00"
                }
                ListElement {
                    status: "stopped"
                    activityColor: "#2196F3"
                    projectName: "Website Redesign"
                    activityName: "Design"
                    description: "Creating wireframes and mockups for client review"
                    duration: "03:20:45"
                    timeRange: "08:30 – 11:50"
                }
                ListElement {
                    status: "stopped"
                    activityColor: "#9C27B0"
                    projectName: "Documentation"
                    activityName: "Writing"
                    description: "API documentation updates and user guide"
                    duration: "00:55:12"
                    timeRange: "07:30 – 08:25"
                }
                ListElement {
                    status: "stopped"
                    activityColor: "#795548"
                    projectName: "Server Maintenance"
                    activityName: "DevOps"
                    description: "Server updates and security patches deployment"
                    duration: "02:10:33"
                    timeRange: "Yesterday 16:00 – 18:10"
                }
            }

            delegate: TimeEntryDelegate {}
        }
    }
}
