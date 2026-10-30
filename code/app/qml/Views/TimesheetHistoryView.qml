// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kemai

import "../Components/HistoryPeriods.js" as HistoryPeriods

Item {
    id: root

    required property SessionContext sessionContext
    required property AppPreferences preferences
    required property UpdateNotifier updates

    // The user chose to log out (and confirmed it if a timer runs)
    property bool settingsAvailable: false
    // History or tasks (Task Management plugin only, switched from the timer hint line); kept for the session's life
    property string view: "history"
    readonly property bool tasksShown: root.sessionContext.hasTaskManagement && root.view === "tasks"

    readonly property TimesheetModel _entries: root.sessionContext.recentTimeSheets
    readonly property bool _narrow: root.width < 640
    // The period named in the header: the one being loaded, else the one shown
    readonly property string _periodKey: root.sessionContext.pendingHistoryPeriod !== "" ? root.sessionContext.pendingHistoryPeriod : root.sessionContext.historyPeriod
    readonly property bool _changingPeriod: root.sessionContext.pendingHistoryPeriod !== ""
    readonly property bool _firstLoad: root.sessionContext.historyLoad === SessionController.NotLoaded || root.sessionContext.historyLoad === SessionController.Loading
    readonly property string _filter: root._entries.filterText.trim()
    // Narrow window: the filter is an icon until opened, then takes the period's place
    property bool _filterOpen: false
    readonly property bool _filterField: !root._narrow || root._filterOpen

    readonly property var _shownRange: root._changingPeriod ? root.sessionContext.historyRangeOf(root._periodKey) : ({
            first: root.sessionContext.historyFirstDate,
            last: root.sessionContext.historyLastDate
        })

    function choosePeriod(key) {
        root.preferences.historyPeriod = key
        root.sessionContext.setHistoryPeriod(key)
    }

    signal logoutRequested
    signal settingsRequested
    signal logJournalRequested
    signal aboutRequested
    // A resource was created from the edit dialog: NotificationKind.ResourceCreated detail
    signal resourceCreated(string detail)

    // Starts a timer at once, or through the dialog when a required custom field would be left empty.
    // `source`: the history entry restarted (its custom values are reused), or null
    function startEntry(activityId, projectId, source) {
        const sourceId = source !== null ? source.timesheetId : 0
        if (root.sessionContext.needsCustomFields(projectId, activityId, sourceId))
            editDialog.openForStart(projectId, activityId, source !== null ? source.customValues : ({}), 0, "")
        else
            root.sessionContext.startTimeSheet(activityId, projectId, sourceId)
    }

    // Started at once, but some custom values could not be written: the dialog shows the entry to retry them
    Connections {
        target: root.sessionContext

        function onCustomFieldsSaveFailed(timesheetId: int, kind: int, detail: string, pendingNames: list<string>) {
            if (!editDialog.opened)
                editDialog.openForUnsaved(timesheetId, kind, pendingNames)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        EntriesHeader {
            Layout.fillWidth: true
            Layout.leftMargin: 16
            Layout.rightMargin: 12
            narrow: root._narrow
            todayDuration: root._entries.todayDuration
            weekDuration: root._entries.weekDuration
            refreshing: root.sessionContext.refreshing
            onRefreshClicked: root.sessionContext.refresh()
            statsVisible: !root.tasksShown
            totalsVisible: !(root._narrow && root._filterOpen)
            middleFills: !root.tasksShown && root._narrow && root._filterOpen

            leading: [
                PeriodSelector {
                    visible: !root.tasksShown && !(root._narrow && root._filterOpen)
                    periodKey: root._periodKey
                    firstDate: root._shownRange.first
                    lastDate: root._shownRange.last
                    loading: root._changingPeriod || root._firstLoad
                    narrow: root._narrow
                    rangeOf: key => root.sessionContext.historyRangeOf(key)
                    onChosen: key => root.choosePeriod(key)
                },
                Text {
                    // Only with room to spare: at the default width the filter needs it
                    visible: !root.tasksShown && root.width >= 880 && root._filter === "" && !root._changingPeriod && root.sessionContext.historyLoad === SessionController.Ready
                    text: "· " + root._entries.totalCount + (root._entries.totalCount === 1 ? " entry" : " entries")
                    color: Theme.colorTextDim
                    font.pixelSize: Theme.fontSizeSmall
                    font.features: Theme.fontFeaturesTabular
                }
            ]

            middle: [
                TaskFilterField {
                    id: historyFilter
                    visible: !root.tasksShown && root._filterField
                    // Shrinks before the header overflows; fills it when opened in a narrow window
                    Layout.fillWidth: true
                    Layout.minimumWidth: 120
                    Layout.maximumWidth: root._narrow ? Number.POSITIVE_INFINITY : 200
                    placeholder: "Filter entries"
                    matchText: root._filter !== "" ? root._entries.count + " of " + root._entries.totalCount : ""
                    text: root._entries.filterText
                    onTextChanged: root._entries.filterText = text
                    onDismissed: root._filterOpen = false
                },
                AbstractButton {
                    id: filterButton
                    visible: !root.tasksShown && !root._filterField
                    Layout.preferredWidth: 30
                    Layout.preferredHeight: 30
                    onClicked: {
                        root._filterOpen = true
                        Qt.callLater(historyFilter.focusInput)
                    }

                    ToolTip.visible: filterButton.hovered
                    ToolTip.delay: 500
                    ToolTip.text: "Filter entries"

                    HoverHandler {
                        cursorShape: Qt.PointingHandCursor
                    }

                    background: Rectangle {
                        radius: Theme.radiusMedium
                        color: root._filter !== "" ? Theme.colorAccentGreenSoft : filterButton.hovered ? Theme.colorSurface : "transparent"
                        border.width: 1
                        border.color: root._filter !== "" ? Theme.colorAccentGreenDim : Theme.colorBorderSubtle
                    }

                    contentItem: Item {
                        Icon {
                            anchors.centerIn: parent
                            name: "search"
                            size: 14
                            color: root._filter !== "" ? Theme.colorAccentGreen : Theme.colorTextSecondary
                        }
                    }
                },
                TaskFilterField {
                    visible: root.tasksShown
                    text: root.sessionContext.tasks.filterText
                    onTextChanged: root.sessionContext.tasks.filterText = text
                },
                AbstractButton {
                    id: tasksRefresh
                    visible: root.tasksShown
                    // Same size as the history's refresh button, at the same place
                    Layout.preferredWidth: 30
                    Layout.preferredHeight: 30
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
                            size: 14
                            color: Theme.colorTextSecondary
                        }
                    }
                }
            ]

            AccountMenu {
                accountName: root.sessionContext.accountName
                host: root.sessionContext.host
                webUrl: root.sessionContext.webUrl
                timerRunning: root.sessionContext.hasActiveTimesheet
                settingsAvailable: root.settingsAvailable
                updates: root.updates
                onSettingsRequested: root.settingsRequested()
                onLogJournalRequested: root.logJournalRequested()
                onAboutRequested: root.aboutRequested()
                onLogoutRequested: root.sessionContext.hasActiveTimesheet ? logoutConfirm.open() : root.logoutRequested()
            }
        }

        TasksPane {
            visible: root.tasksShown
            Layout.fillWidth: true
            Layout.fillHeight: true
            sessionContext: root.sessionContext
            onBackToHistoryRequested: root.view = "history"
            onStartWithFieldsRequested: (taskId, projectId, activityId, title) => editDialog.openForStart(projectId, activityId, ({}), taskId, title)
        }

        Item {
            visible: !root.tasksShown
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                id: entriesList

                WheelScroller {
                    flickable: entriesList
                }
                anchors.fill: parent
                visible: !root._firstLoad
                clip: true
                // The previous period stays while the next one loads
                opacity: root._changingPeriod ? 0.5 : 1
                enabled: !root._changingPeriod

                Behavior on opacity {
                    NumberAnimation {
                        duration: 200
                    }
                }

                model: root._entries

                section.property: "section"
                section.delegate: Item {
                    id: sectionHeader

                    required property string section

                    width: ListView.view ? ListView.view.width : 0
                    height: sectionHeader.section === "" ? 0 : 30

                    Text {
                        visible: sectionHeader.section !== ""
                        anchors.left: parent.left
                        anchors.leftMargin: 16
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 6
                        text: (sectionHeader.section === "before" ? "Running · started before this period" : "In this period").toUpperCase()
                        color: Theme.colorTextLabel
                        font.pixelSize: Theme.fontSizeTiny
                        font.weight: Font.DemiBold
                        font.letterSpacing: 1.2
                    }
                }

                delegate: TimeEntryDelegate {
                    id: entryDelegate

                    highlight: root._filter
                    onStartRequested: (activityId, projectId) => root.startEntry(activityId, projectId, entryDelegate)
                    onEditRequested: editDialog.openFor(entryDelegate)
                }
            }

            // First load of the session: skeleton rows
            Column {
                visible: root._firstLoad
                anchors.fill: parent
                anchors.margins: 12
                spacing: 6

                Repeater {
                    model: [0.9, 0.7, 0.8, 0.6, 0.75]

                    delegate: Rectangle {
                        id: skeletonRow

                        required property real modelData
                        required property int index

                        width: parent ? parent.width : 0
                        height: 52
                        radius: Theme.radiusMedium
                        color: Theme.colorSurfaceDark
                        border.width: 1
                        border.color: Theme.colorBorderSubtle
                        opacity: 1 - skeletonRow.index * 0.15

                        Column {
                            anchors.left: parent.left
                            anchors.leftMargin: 14
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 8

                            Rectangle {
                                width: skeletonRow.width * skeletonRow.modelData * 0.5
                                height: 9
                                radius: 4
                                color: Theme.colorSurface
                            }

                            Rectangle {
                                width: skeletonRow.width * skeletonRow.modelData * 0.7
                                height: 7
                                radius: 4
                                color: Theme.colorSurface
                                opacity: 0.6
                            }
                        }

                        Rectangle {
                            anchors.right: parent.right
                            anchors.rightMargin: 14
                            anchors.verticalCenter: parent.verticalCenter
                            width: 54
                            height: 9
                            radius: 4
                            color: Theme.colorSurface
                        }
                    }
                }
            }

            // Nothing in the period (not an error)
            ColumnLayout {
                visible: root.sessionContext.historyLoad === SessionController.Ready && !root._changingPeriod && root._entries.totalCount === 0
                anchors.centerIn: parent
                width: Math.min(parent.width - 48, 340)
                spacing: 8

                Icon {
                    Layout.alignment: Qt.AlignHCenter
                    name: "calendar"
                    size: 24
                    color: Theme.colorTextSecondary
                }

                Text {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.Wrap
                    text: "Nothing tracked " + HistoryPeriods.phrase(root._periodKey) + " yet"
                    color: Theme.colorTextPrimary
                    font.pixelSize: Theme.fontSizeNormal
                    font.weight: Font.DemiBold
                }

                Text {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.Wrap
                    text: "No entries between " + root._shownRange.first.toLocaleDateString(Qt.locale(), "MMM d") + " and " + root._shownRange.last.toLocaleDateString(Qt.locale(), "MMM d") + ". Start a timer above, or look further back."
                    color: Theme.colorTextSecondary
                    font.pixelSize: Theme.fontSizeSmall
                }

                DialogButton {
                    visible: root._periodKey !== "last30"
                    Layout.alignment: Qt.AlignHCenter
                    Layout.topMargin: 6
                    variant: "subtle"
                    text: "Show last 30 days"
                    onClicked: root.choosePeriod("last30")
                }
            }

            // The filter matches no entry of the period
            ColumnLayout {
                visible: !root._firstLoad && root._filter !== "" && root._entries.totalCount > 0 && root._entries.count === 0
                anchors.centerIn: parent
                width: Math.min(parent.width - 48, 360)
                spacing: 8

                Icon {
                    Layout.alignment: Qt.AlignHCenter
                    name: "search"
                    size: 22
                    color: Theme.colorTextSecondary
                }

                Text {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.Wrap
                    text: "No entries match “" + root._filter + "”"
                    textFormat: Text.PlainText
                    color: Theme.colorTextPrimary
                    font.pixelSize: Theme.fontSizeNormal
                    font.weight: Font.DemiBold
                }

                Text {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.Wrap
                    text: "The filter looks " + HistoryPeriods.scope(root._periodKey) + " (" + HistoryPeriods.dates(root._shownRange.first, root._shownRange.last, Qt.locale()) + ") only: project, activity, description and tags."
                    color: Theme.colorTextSecondary
                    font.pixelSize: Theme.fontSizeSmall
                }

                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.topMargin: 6
                    spacing: 8

                    DialogButton {
                        variant: "subtle"
                        text: "Clear filter"
                        onClicked: root._entries.filterText = ""
                    }

                    DialogButton {
                        visible: root._periodKey !== "last30"
                        variant: "ghost"
                        text: "Search last 30 days"
                        onClicked: root.choosePeriod("last30")
                    }
                }
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
