// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kemai

// Tasks assigned to the user (Task Management plugin): grouped list, start / close per task, and the
// loading, empty, no-match and failed states. Loads the tasks whenever it becomes visible.
Item {
    id: root

    required property SessionContext sessionContext

    signal backToHistoryRequested

    readonly property TaskModel _tasks: root.sessionContext.tasks
    readonly property int _load: root.sessionContext.tasksLoad
    readonly property string _filter: root._tasks.filterText.trim()

    // Per task: "starting" | "closing"
    property var _pending: ({})
    // Task asking for close confirmation, 0 if none
    property int _confirmClose: 0
    // Per task: { action: "start" | "close", kind }
    property var _rowErrors: ({})
    // Task ids whose todo text is open
    property var _expanded: ({})

    function _set(map, key, value) {
        const copy = Object.assign({}, map)
        if (value === undefined)
            delete copy[key]
        else
            copy[key] = value
        return copy
    }

    function _start(taskId) {
        if (root._pending[taskId] !== undefined)
            return
        root._rowErrors = root._set(root._rowErrors, taskId, undefined)
        root._pending = root._set(root._pending, taskId, "starting")
        root.sessionContext.startTask(taskId)
    }

    function _close(taskId) {
        if (root._pending[taskId] !== undefined)
            return
        root._rowErrors = root._set(root._rowErrors, taskId, undefined)
        root._pending = root._set(root._pending, taskId, "closing")
        root.sessionContext.closeTask(taskId)
    }

    function _retry(taskId) {
        const error = root._rowErrors[taskId]
        if (error.action === "start")
            root._start(taskId)
        else
            root._close(taskId)
    }

    function _errorText(error) {
        const verb = error.action === "start" ? "Couldn't start —" : "Couldn't close —"
        let detail
        switch (error.kind) {
        case SessionController.NoNetwork:
            detail = "no connection. Nothing changed."
            break
        case SessionController.Forbidden:
            detail = "permission refused by Kimai."
            break
        case SessionController.NotFound:
            detail = "the task no longer exists. Refresh the list."
            break
        case SessionController.ServerError:
            detail = error.action === "close" ? "server error. The task stays open." : "server error. Nothing changed."
            break
        case SessionController.InvalidResponse:
            detail = "unexpected response from Kimai."
            break
        default:
            detail = error.detail !== "" ? error.detail : "Kimai refused it."
        }
        return "<b>" + verb + "</b> <font color=\"" + Theme.colorTextSecondary + "\">" + detail + "</font>"
    }

    function _duration(seconds) {
        const hours = Math.floor(seconds / 3600)
        const minutes = Math.round((seconds % 3600) / 60)
        if (hours === 0)
            return minutes + "m"
        return minutes === 0 ? hours + "h" : hours + "h " + minutes + "m"
    }

    function _highlight(text, query) {
        const esc = s => s.replace(/&/g, "&amp;").replace(/</g, "&lt;")
        const i = query === "" ? -1 : text.toLowerCase().indexOf(query.toLowerCase())
        if (i < 0)
            return esc(text)
        return esc(text.slice(0, i)) + "<font color='" + Theme.colorAccentGreen + "'>" + esc(text.slice(i, i + query.length)) + "</font>" + esc(text.slice(i + query.length))
    }

    onVisibleChanged: if (root.visible)
        root.sessionContext.loadTasks()
    Component.onCompleted: if (root.visible)
        root.sessionContext.loadTasks()

    Connections {
        target: root.sessionContext
        function onTaskStarted(taskId) {
            root._pending = root._set(root._pending, taskId, undefined)
        }
        function onTaskClosed(taskId) {
            root._pending = root._set(root._pending, taskId, undefined)
            if (root._confirmClose === taskId)
                root._confirmClose = 0
        }
        function onTaskActionFailed(taskId, action, error, detail) {
            root._pending = root._set(root._pending, taskId, undefined)
            if (root._confirmClose === taskId)
                root._confirmClose = 0
            root._rowErrors = root._set(root._rowErrors, taskId, {
                action: action,
                kind: error,
                detail: detail
            })
        }
    }

    // -- List
    ListView {
        id: taskList

        WheelScroller {
            flickable: taskList
        }

        anchors.fill: parent
        visible: root._load === SessionController.Ready && taskList.count > 0
        clip: true
        model: root._tasks
        boundsBehavior: Flickable.StopAtBounds

        section.property: "group"
        section.delegate: Rectangle {
            id: groupHeader

            required property string section

            width: ListView.view.width
            height: groupRow.implicitHeight + 16
            color: Theme.colorBackgroundDark

            RowLayout {
                id: groupRow
                anchors.left: parent.left
                anchors.leftMargin: 16
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 6
                spacing: 6

                Text {
                    text: ({
                            progress: "In progress",
                            overdue: "Overdue",
                            upcoming: "Upcoming",
                            none: "No due date",
                            matches: "Matches"
                        })[groupHeader.section].toUpperCase()
                    color: groupHeader.section === "overdue" ? Theme.colorAccentRed : Theme.colorTextLabel
                    font.pixelSize: Theme.fontSizeTiny
                    font.weight: Font.DemiBold
                    font.letterSpacing: 1.3
                }

                Text {
                    // Re-evaluated when the shown rows change (count)
                    text: root._tasks.count >= 0 ? root._tasks.groupCount(groupHeader.section) : 0
                    color: Theme.colorTextDim
                    font.pixelSize: Theme.fontSizeTiny
                    font.features: Theme.fontFeaturesTabular
                }
            }
        }

        delegate: ColumnLayout {
            id: row

            required property int taskId
            required property string title
            required property string todo
            required property string status
            required property bool running
            required property string projectName
            required property string activityName
            required property string activityColor
            required property date dueAt
            required property string dueState
            required property int overdueDays
            required property int estimation

            readonly property string pending: root._pending[row.taskId] !== undefined ? root._pending[row.taskId] : ""
            readonly property bool confirming: root._confirmClose === row.taskId
            readonly property var error: root._rowErrors[row.taskId] !== undefined ? root._rowErrors[row.taskId] : null
            readonly property bool expanded: root._expanded[row.taskId] === true
            property bool spinnerShown: false

            onPendingChanged: {
                row.spinnerShown = false
                if (row.pending !== "")
                    spinnerDelay.restart()
            }

            width: ListView.view.width
            spacing: 0

            Timer {
                id: spinnerDelay
                interval: 150
                onTriggered: row.spinnerShown = true
            }

            // ---- Task line
            Rectangle {
                visible: !row.confirming
                Layout.fillWidth: true
                Layout.preferredHeight: taskLine.implicitHeight + 20
                color: row.running ? Theme.colorRowRunning : "transparent"

                Rectangle {
                    x: 0
                    y: 8
                    width: 3
                    height: parent.height - 16
                    radius: 2
                    color: row.activityColor
                }

                RowLayout {
                    id: taskLine
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16
                    spacing: 12

                    // Expand chevron (todo text only); the column stays to keep titles aligned
                    Item {
                        Layout.preferredWidth: 16
                        Layout.preferredHeight: 16

                        Icon {
                            visible: row.todo !== ""
                            anchors.centerIn: parent
                            name: row.expanded ? "chevron-down" : "chevron-right"
                            size: 13
                            color: Theme.colorTextSecondary

                            TapHandler {

                                gesturePolicy: TapHandler.ReleaseWithinBounds
                                onTapped: root._expanded = root._set(root._expanded, row.taskId, row.expanded ? undefined : true)
                            }

                            HoverHandler {
                                cursorShape: Qt.PointingHandCursor
                            }
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 3

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            // Takes its full width first; elided only when the row is too narrow
                            Text {
                                Layout.fillWidth: true
                                // Rounded up: a layout width a fraction below implicitWidth elides the text
                                Layout.preferredWidth: Math.ceil(implicitWidth)
                                Layout.maximumWidth: Math.ceil(implicitWidth)
                                text: root._highlight(row.title, root._filter)
                                textFormat: Text.StyledText
                                color: Theme.colorTextPrimary
                                font.pixelSize: Theme.fontSizeMedium
                                font.weight: Font.DemiBold
                                elide: Text.ElideRight
                            }

                            // RUNNING / IN PROGRESS badge (pending tasks get none)
                            Rectangle {
                                visible: row.running || row.status === "progress"
                                Layout.preferredWidth: badgeRow.implicitWidth + (row.running ? 16 : 14)
                                Layout.preferredHeight: badgeRow.implicitHeight + 4
                                radius: height / 2
                                color: row.running ? Theme.colorAccentGreen : "transparent"
                                border.width: row.running ? 0 : 1
                                border.color: Theme.colorAccentGreenDim

                                RowLayout {
                                    id: badgeRow
                                    anchors.centerIn: parent
                                    spacing: 5

                                    Rectangle {
                                        visible: row.running
                                        Layout.preferredWidth: 5
                                        Layout.preferredHeight: 5
                                        radius: 2.5
                                        color: Theme.colorTextOnAccent
                                    }

                                    Text {
                                        text: row.running ? "RUNNING" : "IN PROGRESS"
                                        color: row.running ? Theme.colorTextOnAccent : Theme.colorAccentGreen
                                        font.pixelSize: Theme.fontSizeTiny
                                        font.weight: Font.DemiBold
                                        font.letterSpacing: 0.6
                                    }
                                }
                            }

                            Item {
                                Layout.fillWidth: true
                                Layout.preferredWidth: 0
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 6

                            Rectangle {
                                Layout.preferredWidth: 8
                                Layout.preferredHeight: 8
                                radius: 2
                                color: row.activityColor
                            }

                            Text {
                                text: row.projectName
                                color: Theme.colorTextPrimary
                                font.pixelSize: Theme.fontSizeSmall
                            }

                            Text {
                                text: "—"
                                color: Theme.colorTextDim
                                font.pixelSize: Theme.fontSizeSmall
                            }

                            Text {
                                Layout.fillWidth: true
                                // Rounded up: a layout width a fraction below implicitWidth elides the text
                                Layout.preferredWidth: Math.ceil(implicitWidth)
                                Layout.maximumWidth: Math.ceil(implicitWidth)
                                text: row.activityName
                                color: Theme.colorTextSecondary
                                font.pixelSize: Theme.fontSizeSmall
                                elide: Text.ElideRight
                            }

                            Text {
                                text: "·"
                                color: Theme.colorTextDim
                                font.pixelSize: Theme.fontSizeSmall
                            }

                            Icon {
                                name: row.dueState === "overdue" ? "circle-alert" : "calendar"
                                size: 11
                                color: row.dueState === "overdue" ? Theme.colorAccentRed : row.dueState === "today" ? Theme.colorWarning : row.dueState === "none" ? Theme.colorTextDim : Theme.colorTextSecondary
                            }

                            Text {
                                Layout.fillWidth: true
                                Layout.preferredWidth: 0
                                Layout.minimumWidth: implicitWidth
                                text: row.dueState === "none" ? "No due date" : row.dueState === "today" ? "Due today" : row.dueState === "overdue" ? "Overdue · " + row.overdueDays + " d" : Qt.formatDate(row.dueAt, "ddd d MMM")
                                color: row.dueState === "overdue" ? Theme.colorAccentRed : row.dueState === "today" ? Theme.colorWarning : row.dueState === "none" ? Theme.colorTextDim : Theme.colorTextSecondary
                                font.pixelSize: Theme.fontSizeXSmall
                                font.weight: row.dueState === "overdue" || row.dueState === "today" ? Font.DemiBold : Font.Normal
                                font.features: Theme.fontFeaturesTabular
                                elide: Text.ElideRight
                            }
                        }
                    }

                    // Estimate (no time spent: Kimai's task API does not give it)
                    Text {
                        Layout.preferredWidth: 92
                        horizontalAlignment: Text.AlignRight
                        text: row.estimation > 0 ? "est. " + root._duration(row.estimation) : ""
                        color: Theme.colorTextSecondary
                        font.pixelSize: Theme.fontSizeXSmall
                        font.features: Theme.fontFeaturesTabular
                    }

                    // Close
                    AbstractButton {
                        id: closeButton
                        Layout.preferredWidth: 28
                        Layout.preferredHeight: 28
                        enabled: row.pending === ""
                        onClicked: {
                            root._rowErrors = root._set(root._rowErrors, row.taskId, undefined)
                            root._confirmClose = row.taskId
                        }

                        ToolTip.visible: closeButton.hovered
                        ToolTip.delay: 500
                        ToolTip.text: "Close task"

                        HoverHandler {
                            cursorShape: Qt.PointingHandCursor
                        }

                        background: Rectangle {
                            radius: Theme.radiusMedium
                            color: closeButton.hovered ? Theme.colorSurface : "transparent"
                            border.width: 1
                            border.color: Theme.colorBorderSubtle
                        }

                        contentItem: Item {
                            Icon {
                                anchors.centerIn: parent
                                name: "check"
                                size: 14
                                color: Theme.colorTextSecondary
                            }
                        }
                    }

                    // Start, or the running well (stopping is done from the timer bar)
                    AbstractButton {
                        id: startButton
                        visible: !row.running
                        Layout.preferredWidth: 28
                        Layout.preferredHeight: 28
                        onClicked: root._start(row.taskId)

                        ToolTip.visible: startButton.hovered && row.pending === ""
                        ToolTip.delay: 500
                        ToolTip.text: "Start"

                        HoverHandler {
                            cursorShape: row.pending === "" ? Qt.PointingHandCursor : Qt.WaitCursor
                        }

                        background: Rectangle {
                            radius: Theme.radiusMedium
                            color: Theme.colorAccentGreenSoft
                            border.width: 1
                            border.color: startButton.hovered ? Theme.colorAccentGreen : Theme.colorAccentGreenDim
                        }

                        contentItem: Item {
                            Icon {
                                visible: !(row.pending === "starting" && row.spinnerShown)
                                anchors.centerIn: parent
                                name: "play"
                                size: 13
                                color: Theme.colorAccentGreen
                            }

                            Spinner {
                                visible: row.pending === "starting" && row.spinnerShown
                                anchors.centerIn: parent
                                size: 12
                                color: Theme.colorAccentGreen
                                trackColor: Theme.colorBorderSubtle
                            }
                        }
                    }

                    Item {
                        visible: row.running
                        Layout.preferredWidth: 28
                        Layout.preferredHeight: 28

                        DashedBorder {
                            anchors.fill: parent
                            radius: Theme.radiusMedium
                            color: Theme.colorAccentGreenDim
                        }

                        Rectangle {
                            anchors.centerIn: parent
                            width: 13
                            height: 13
                            radius: 6.5
                            color: Theme.colorAccentGreenSoft

                            Rectangle {
                                anchors.centerIn: parent
                                width: 7
                                height: 7
                                radius: 3.5
                                color: Theme.colorAccentGreen
                            }
                        }
                    }
                }
            }

            // ---- Close confirmation (replaces the line)
            Rectangle {
                visible: row.confirming
                Layout.fillWidth: true
                Layout.preferredHeight: confirmRow.implicitHeight + 20
                color: Theme.colorSurfaceDark

                Rectangle {
                    width: 3
                    height: parent.height
                    color: Theme.colorAccentGreen
                }

                Keys.onEscapePressed: root._confirmClose = 0

                RowLayout {
                    id: confirmRow
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16
                    spacing: 12

                    Icon {
                        name: "check"
                        size: 16
                        color: Theme.colorAccentGreen
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2

                        Text {
                            Layout.fillWidth: true
                            text: "Close “" + row.title + "”?"
                            color: Theme.colorTextPrimary
                            font.pixelSize: Theme.fontSizeMedium
                            font.weight: Font.DemiBold
                            elide: Text.ElideRight
                        }

                        Text {
                            Layout.fillWidth: true
                            // Checked on Kimai 2.70: closing a task also stops its running timer
                            text: row.running ? "It's marked done in Kimai, which also stops its timer." : "It's marked done in Kimai and leaves this list."
                            color: Theme.colorTextSecondary
                            font.pixelSize: Theme.fontSizeSmall
                            elide: Text.ElideRight
                        }
                    }

                    DialogButton {
                        id: keepButton
                        text: "Keep open"
                        enabled: row.pending === ""
                        onClicked: root._confirmClose = 0
                        Component.onCompleted: if (row.confirming)
                            keepButton.forceActiveFocus()
                        Keys.onEscapePressed: root._confirmClose = 0
                    }

                    DialogButton {
                        Layout.minimumWidth: 96
                        variant: "primary"
                        text: row.pending === "closing" && row.spinnerShown ? "Closing…" : "Close task"
                        busy: row.pending === "closing"
                        spinning: row.spinnerShown
                        onClicked: root._close(row.taskId)
                    }
                }

                onVisibleChanged: if (visible)
                    keepButton.forceActiveFocus()
            }

            // ---- Todo text
            Item {
                visible: row.expanded && row.todo !== "" && !row.confirming
                Layout.fillWidth: true
                Layout.preferredHeight: todoBox.height + 12

                Rectangle {
                    id: todoBox
                    x: 56
                    width: parent.width - 56 - 16
                    height: todoText.implicitHeight + 16
                    radius: Theme.radiusMedium
                    color: Theme.colorBackgroundDark
                    border.width: 1
                    border.color: Theme.colorBorderSubtle

                    Text {
                        id: todoText
                        anchors.fill: parent
                        anchors.margins: 8
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12
                        text: row.todo
                        color: Theme.colorTextSecondary
                        font.pixelSize: Theme.fontSizeSmall
                        lineHeight: 1.3
                        wrapMode: Text.Wrap
                    }
                }
            }

            // ---- Failed action
            Item {
                visible: row.error !== null
                Layout.fillWidth: true
                Layout.preferredHeight: errorStrip.height + 10

                Rectangle {
                    id: errorStrip
                    x: 56
                    width: parent.width - 56 - 16
                    height: errorRow.implicitHeight + 14
                    radius: Theme.radiusMedium
                    color: Theme.colorAccentRedSoft
                    border.width: 1
                    border.color: Theme.colorAccentRedLine

                    RowLayout {
                        id: errorRow
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10
                        spacing: 8

                        Icon {
                            name: row.error !== null && row.error.kind === SessionController.NoNetwork ? "wifi-off" : "circle-alert"
                            size: 13
                            color: Theme.colorAccentRed
                        }

                        Text {
                            Layout.fillWidth: true
                            textFormat: Text.StyledText
                            text: row.error !== null ? root._errorText(row.error) : ""
                            color: Theme.colorTextPrimary
                            font.pixelSize: Theme.fontSizeSmall
                            wrapMode: Text.Wrap
                        }

                        DialogButton {
                            text: "Retry"
                            onClicked: root._retry(row.taskId)
                        }

                        Icon {
                            name: "x"
                            size: 12
                            color: Theme.colorTextSecondary

                            TapHandler {

                                gesturePolicy: TapHandler.ReleaseWithinBounds
                                onTapped: root._rowErrors = root._set(root._rowErrors, row.taskId, undefined)
                            }

                            HoverHandler {
                                cursorShape: Qt.PointingHandCursor
                            }
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: Theme.colorDivider
            }
        }
    }

    // -- First load: skeleton
    ColumnLayout {
        visible: root._load === SessionController.Loading || root._load === SessionController.NotLoaded
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 26
            color: Theme.colorBackgroundDark

            Rectangle {
                x: 16
                y: 11
                width: 80
                height: 8
                radius: 3
                color: Theme.colorSurfaceDark
            }
        }

        Repeater {
            model: [0.62, 0.45, 0.7, 0.38, 0.55]

            delegate: Item {
                id: skeletonRow

                required property real modelData
                required property int index

                Layout.fillWidth: true
                Layout.preferredHeight: 58
                opacity: 1 - skeletonRow.index * 0.15

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 44
                    anchors.rightMargin: 16
                    spacing: 12

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Rectangle {
                            Layout.preferredWidth: parent.width * skeletonRow.modelData
                            Layout.preferredHeight: 10
                            radius: 3
                            color: Theme.colorSurfaceDark
                        }

                        Rectangle {
                            Layout.preferredWidth: parent.width * 0.34
                            Layout.preferredHeight: 8
                            radius: 3
                            color: Theme.colorSurfaceDark
                        }
                    }

                    Rectangle {
                        Layout.preferredWidth: 92
                        Layout.preferredHeight: 8
                        radius: 3
                        color: Theme.colorSurfaceDark
                    }

                    Rectangle {
                        Layout.preferredWidth: 28
                        Layout.preferredHeight: 28
                        radius: Theme.radiusMedium
                        color: Theme.colorSurfaceDark
                    }

                    Rectangle {
                        Layout.preferredWidth: 28
                        Layout.preferredHeight: 28
                        radius: Theme.radiusMedium
                        color: Theme.colorSurfaceDark
                    }
                }
            }
        }
    }

    // -- Empty, no match, failed
    ColumnLayout {
        id: stateColumn

        readonly property string kind: root._load === SessionController.Failed ? "failed" : root._load !== SessionController.Ready ? "" : root._tasks.totalCount === 0 ? "empty" : taskList.count === 0 ? "nomatch" : ""

        visible: stateColumn.kind !== ""
        anchors.centerIn: parent
        width: Math.min(parent.width - 40, 380)
        spacing: 0

        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 44
            Layout.preferredHeight: 44
            radius: 12
            color: stateColumn.kind === "failed" ? Theme.colorAccentRedSoft : Theme.colorSurfaceDark
            border.width: 1
            border.color: stateColumn.kind === "failed" ? Theme.colorAccentRedLine : Theme.colorBorderSubtle

            Icon {
                anchors.centerIn: parent
                name: stateColumn.kind === "failed" ? (root.sessionContext.tasksError === SessionController.NoNetwork ? "wifi-off" : "circle-alert") : stateColumn.kind === "nomatch" ? "search" : "check"
                size: 20
                color: stateColumn.kind === "failed" ? Theme.colorAccentRed : Theme.colorTextSecondary
            }
        }

        Text {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 14
            text: stateColumn.kind === "failed" ? "Couldn't load tasks" : stateColumn.kind === "nomatch" ? "No task matches “" + root._filter + "”" : "No open tasks"
            color: Theme.colorTextPrimary
            font.pixelSize: Theme.fontSizeNormal
            font.weight: Font.DemiBold
        }

        Text {
            Layout.fillWidth: true
            Layout.maximumWidth: 340
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 6
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            text: stateColumn.kind === "failed" ? (root.sessionContext.tasksError === SessionController.NoNetwork ? "No connection to " + root.sessionContext.host + ". Your history and the timer still work." : "Kimai answered with an error. Your history and the timer still work.") : stateColumn.kind === "nomatch" ? "The filter looks in titles, projects, activities and todo text." : "Tasks assigned to you in Kimai appear here. Nothing to do right now."
            color: Theme.colorTextSecondary
            font.pixelSize: Theme.fontSizeSmall
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 16
            spacing: 8

            DialogButton {
                visible: stateColumn.kind === "failed"
                variant: "primary"
                text: "Retry"
                onClicked: root.sessionContext.loadTasks()
            }

            DialogButton {
                text: stateColumn.kind === "nomatch" ? "Clear filter" : "Back to history"
                onClicked: {
                    if (stateColumn.kind === "nomatch")
                        root._tasks.filterText = ""
                    else
                        root.backToHistoryRequested()
                }
            }
        }
    }
}
