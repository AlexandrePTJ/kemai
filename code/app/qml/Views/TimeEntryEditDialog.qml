// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kemai

// Edits one time entry (project — activity, description, start/end, tags) or deletes it.
// Edits stay local until Save; save and delete failures are shown here, with the edits kept.
Popup {
    id: root

    required property SessionContext sessionContext

    // A resource was created from the project — activity search: NotificationKind.ResourceCreated detail
    signal resourceCreated(string detail)

    // Entry as opened
    property int timesheetId: 0
    property int _origProjectId: 0
    property int _origActivityId: 0
    property string _origDescription: ""
    property var _origTags: []
    property date _origBegin: new Date()
    property date _origEnd: new Date()
    property string _origProjectName: ""
    property string _origDuration: ""
    property bool running: false

    // Draft
    property int _projectId: 0
    property int _activityId: 0
    property string _projectName: ""
    property string _activityName: ""
    property string _activityColor: "transparent"
    property date _begin: new Date()
    property date _end: new Date()
    property date _now: new Date()

    // idle | saving | confirmDelete | deleting
    property string _phase: "idle"
    // null, or { op: "save" | "delete", kind, detail }
    property var _error: null
    property bool _spinnerShown: false

    readonly property bool _punch: root.sessionContext.punchMode
    readonly property bool _busy: root._phase === "saving" || root._phase === "deleting"
    readonly property bool _confirming: root._phase === "confirmDelete" || root._phase === "deleting"

    readonly property bool _projectChanged: root._projectId !== root._origProjectId || root._activityId !== root._origActivityId
    readonly property bool _descriptionChanged: descriptionEdit.text !== root._origDescription
    readonly property bool _timeChanged: !root._punch && (root._begin.getTime() !== root._origBegin.getTime() || (!root.running && root._end.getTime() !== root._origEnd.getTime()))
    readonly property bool _tagsChanged: !root._sameSet(tagsInput.tags, root._origTags)
    readonly property bool _dirty: root._projectChanged || root._descriptionChanged || root._timeChanged || root._tagsChanged

    readonly property bool _rangeValid: root.running || root._punch || root._end.getTime() > root._begin.getTime()
    readonly property bool _canSave: root._dirty && root._rangeValid && root._phase === "idle"

    // Opens the dialog on one entry of the history (TimesheetModel roles)
    function openFor(entry) {
        root.timesheetId = entry.timesheetId
        root.running = entry.status === "active"
        root._origProjectId = entry.projectId
        root._origActivityId = entry.activityId
        root._origDescription = entry.description
        root._origTags = entry.tags ? Array.from(entry.tags) : []
        root._origBegin = entry.beginAt
        root._origEnd = entry.endAt
        root._origProjectName = entry.projectName
        root._origDuration = root._formatDuration(entry.beginAt, root.running ? new Date() : entry.endAt)

        root._projectId = entry.projectId
        root._activityId = entry.activityId
        root._projectName = entry.projectName
        root._activityName = entry.activityName
        root._activityColor = entry.activityColor
        root._begin = entry.beginAt
        root._end = entry.endAt
        root._now = new Date()
        descriptionEdit.text = entry.description
        tagsInput.tags = Array.from(root._origTags)
        tagsInput.newTags = []

        root._phase = "idle"
        root._error = null
        root.open()
    }

    function _sameSet(a, b) {
        if (a.length !== b.length)
            return false
        const lower = l => l.map(t => t.toLowerCase()).sort().join("\n")
        return lower(a) === lower(b)
    }

    function _pad2(n) {
        return n < 10 ? "0" + n : "" + n
    }

    function _formatDuration(from, to) {
        const minutes = Math.max(0, Math.floor((to.getTime() - from.getTime()) / 60000))
        return Math.floor(minutes / 60) + "h " + root._pad2(minutes % 60) + "m"
    }

    function _formatTime(d) {
        return root._pad2(d.getHours()) + ":" + root._pad2(d.getMinutes())
    }

    // "Today", "Yesterday", or "" for other days
    function _dayWord(d) {
        const day = new Date(d.getFullYear(), d.getMonth(), d.getDate())
        const now = new Date()
        const today = new Date(now.getFullYear(), now.getMonth(), now.getDate())
        const diffDays = Math.round((today.getTime() - day.getTime()) / 86400000)
        if (diffDays === 0)
            return "Today"
        if (diffDays === 1)
            return "Yesterday"
        return ""
    }

    function _dateLabel(d) {
        const word = root._dayWord(d)
        const date = d.toLocaleDateString(Qt.locale(), "ddd d MMM")
        return word !== "" ? word + ", " + date : date
    }

    function _shortPoint(d) {
        const word = root._dayWord(d)
        return (word !== "" ? word : d.toLocaleDateString(Qt.locale(), "ddd d MMM")) + " " + root._formatTime(d)
    }

    function _save() {
        if (!root._rangeValid || root._busy)
            return
        root._error = null
        // Invalid dates leave begin/end unchanged: never sent in punch mode, and a running entry has no end
        const begin = root._punch ? new Date(NaN) : root._begin
        const end = root._punch || root.running ? new Date(NaN) : root._end
        if (root.sessionContext.updateTimeSheet(root.timesheetId, root._projectId, root._activityId, descriptionEdit.text, tagsInput.tags, begin, end, tagsInput.newTags)) {
            root._startPhase("saving")
        } else {
            root._error = {
                op: "save",
                kind: SessionController.UnknownError,
                detail: ""
            }
        }
    }

    function _delete() {
        root._error = null
        if (root.sessionContext.deleteTimeSheet(root.timesheetId))
            root._startPhase("deleting")
        else
            root._phase = "idle"
    }

    function _startPhase(phase) {
        root._phase = phase
        root._spinnerShown = false
        spinnerDelay.restart()
    }

    function _cancel() {
        if (root._busy)
            return
        // The entry vanished meanwhile: show the server's list again
        if (root._error && root._error.kind === SessionController.NotFound)
            root.sessionContext.refresh()
        root.close()
    }

    // Innermost layer first: the fields close their own dropdowns, then the delete confirmation, then the dialog
    function _escape() {
        if (root._phase === "confirmDelete")
            root._phase = "idle"
        else
            root._cancel()
    }

    function _errorTitle(error) {
        const prefix = error.op === "delete" ? "Couldn't delete" : "Couldn't save"
        switch (error.kind) {
        case SessionController.NoNetwork:
            return prefix + " — no connection"
        case SessionController.Forbidden:
            return prefix + " — permission refused"
        case SessionController.NotFound:
            return prefix + " — entry no longer exists"
        case SessionController.ServerError:
            return prefix + " — server error"
        case SessionController.InvalidResponse:
            return prefix + " — unexpected response"
        default:
            return prefix + " — Kimai refused the change"
        }
    }

    function _errorDetail(error) {
        switch (error.kind) {
        case SessionController.NoNetwork:
            return "Your changes are kept. Check the network, then retry."
        case SessionController.Forbidden:
            return "Your Kimai account isn't allowed to " + (error.op === "delete" ? "delete" : "edit") + " this entry. Cancel to discard."
        case SessionController.NotFound:
            return "It was deleted on the server. Cancel to refresh the list."
        case SessionController.ServerError:
            return "Kimai returned an error. Your changes are kept; retry in a moment."
        case SessionController.InvalidResponse:
            return "Kimai answered in a way Kemai doesn’t understand. Your changes are kept."
        default:
            return error.detail !== "" ? error.detail : "Your changes are kept."
        }
    }

    // Centered over the whole window, not the history list it is declared in
    parent: Overlay.overlay
    modal: true
    focus: true
    closePolicy: Popup.NoAutoClose
    width: 540
    height: dialogColumn.implicitHeight
    x: parent ? (parent.width - width) / 2 : 0
    y: parent ? (parent.height - height) / 2 : 0
    padding: 0

    Overlay.modal: Rectangle {
        color: Theme.colorBackdrop
    }

    background: Rectangle {
        color: Theme.colorBackground
        border.color: Theme.colorBorderSubtle
        border.width: 1
        radius: Theme.radiusDialog
    }

    Connections {
        target: root.sessionContext

        function onTimeSheetSaved(timesheetId: int) {
            if (timesheetId === root.timesheetId && root._phase === "saving")
                root.close()
        }
        function onTimeSheetSaveFailed(timesheetId: int, kind: int, detail: string) {
            if (timesheetId !== root.timesheetId || root._phase !== "saving")
                return
            root._phase = "idle"
            root._error = {
                op: "save",
                kind: kind,
                detail: detail
            }
        }
        function onTimeSheetDeleted(timesheetId: int) {
            if (timesheetId === root.timesheetId && root._phase === "deleting")
                root.close()
        }
        function onTimeSheetDeleteFailed(timesheetId: int, kind: int, detail: string) {
            if (timesheetId !== root.timesheetId || root._phase !== "deleting")
                return
            root._phase = "idle"
            root._error = {
                op: "delete",
                kind: kind,
                detail: detail
            }
        }
        // The app goes back to login: don't stand in the way
        function onSessionExpired() {
            root.close()
        }
    }

    // Spinner only for requests that last: fast ones don't flash it
    // Creation of a project or activity the search did not find; the new pair replaces the entry's
    ResourceCreationDialog {
        id: creationDialog
        sessionContext: root.sessionContext
        onPairChosen: pair => {
            root._projectId = pair.projectId
            root._activityId = pair.activityId
            root._projectName = pair.projectName
            root._activityName = pair.activityName
            root._activityColor = pair.activityColor
            descriptionEdit.forceActiveFocus()
        }
        onCreated: detail => root.resourceCreated(detail)
    }

    Timer {
        id: spinnerDelay
        interval: 150
        onTriggered: root._spinnerShown = true
    }

    // Running entry: duration up to now
    Timer {
        interval: 1000
        repeat: true
        running: root.opened && root.running
        onTriggered: root._now = new Date()
    }

    Shortcut {
        sequences: ["Ctrl+Return", "Ctrl+Enter"]
        enabled: root.opened && root._canSave && !picker.opened
        onActivated: root._save()
    }

    DateTimeEditDialog {
        id: picker

        property bool editEnd: false

        parent: Overlay.overlay
        onConfirmed: dateTime => {
            if (editEnd)
                root._end = dateTime
            else
                root._begin = dateTime
        }
    }

    component FieldLabel: RowLayout {
        id: fieldLabel

        property string text
        property bool changed: false

        spacing: 8

        Text {
            text: fieldLabel.text
            color: Theme.colorTextLabel
            font.pixelSize: Theme.fontSizeXSmall
            font.weight: Font.DemiBold
            font.letterSpacing: 1.3
            font.capitalization: Font.AllUppercase
        }

        RowLayout {
            visible: fieldLabel.changed
            spacing: 4

            Rectangle {
                Layout.preferredWidth: 5
                Layout.preferredHeight: 5
                radius: 2.5
                color: Theme.colorAccentGreen
            }

            Text {
                text: "modified"
                color: Theme.colorAccentGreen
                font.pixelSize: Theme.fontSizeTiny
                font.weight: Font.Medium
            }
        }
    }

    component TimeChip: FocusScope {
        id: chip

        property string which
        property date value
        // normal | error | locked | now
        property string mode: "normal"
        readonly property bool locked: chip.mode === "locked" || chip.mode === "now"

        signal pickRequested

        Layout.fillWidth: true
        Layout.preferredHeight: 46
        activeFocusOnTab: !chip.locked

        Keys.onPressed: event => {
            if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key === Qt.Key_Space) {
                chip.pickRequested()
                event.accepted = true
            }
        }

        Rectangle {
            anchors.fill: parent
            radius: Theme.radiusMedium
            color: chip.locked ? "transparent" : Theme.colorSurface
            border.width: chip.mode === "error" || chip.activeFocus ? 1 : 0
            border.color: chip.mode === "error" ? Theme.colorAccentRed : Theme.colorAccentGreenDim

            DashedBorder {
                visible: chip.locked
            }

            TapHandler {

                gesturePolicy: TapHandler.ReleaseWithinBounds
                enabled: !chip.locked
                onTapped: chip.pickRequested()
            }

            HoverHandler {
                enabled: !chip.locked
                cursorShape: Qt.PointingHandCursor
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                spacing: 10

                Icon {
                    name: chip.locked ? "lock" : "clock"
                    size: 15
                    color: chip.mode === "error" ? Theme.colorAccentRed : Theme.colorTextSecondary
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 1

                    Text {
                        Layout.fillWidth: true
                        text: chip.which + " · " + (chip.mode === "now" ? "while running" : root._dateLabel(chip.value))
                        color: Theme.colorTextSecondary
                        font.pixelSize: Theme.fontSizeXSmall
                        elide: Text.ElideRight
                    }

                    Text {
                        text: chip.mode === "now" ? "now" : root._formatTime(chip.value)
                        color: chip.mode === "error" ? Theme.colorAccentRed : chip.mode === "now" ? Theme.colorAccentGreen : chip.mode === "locked" ? Theme.colorTextSecondary : Theme.colorTextPrimary
                        font.pixelSize: Theme.fontSizeTitle
                        font.weight: Font.DemiBold
                        font.features: Theme.fontFeaturesTabular
                    }
                }
            }
        }
    }

    FocusScope {
        anchors.fill: parent
        focus: true

        Keys.onEscapePressed: root._escape()

        ColumnLayout {
            id: dialogColumn
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 0

            // -- Header
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 44

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 18
                    anchors.rightMargin: 18
                    spacing: 10

                    Rectangle {
                        Layout.preferredWidth: 4
                        Layout.preferredHeight: 18
                        radius: 2
                        color: root._activityColor
                    }

                    Text {
                        text: "Edit time entry"
                        color: Theme.colorTextPrimary
                        font.pixelSize: Theme.fontSizeNormal
                        font.weight: Font.DemiBold
                    }

                    Rectangle {
                        visible: root.running
                        Layout.preferredWidth: runningRow.implicitWidth + 16
                        Layout.preferredHeight: 18
                        radius: 9
                        color: Theme.colorAccentGreen

                        RowLayout {
                            id: runningRow
                            anchors.centerIn: parent
                            spacing: 5

                            Rectangle {
                                Layout.preferredWidth: 5
                                Layout.preferredHeight: 5
                                radius: 2.5
                                color: Theme.colorTextOnAccent
                            }

                            Text {
                                text: "RUNNING"
                                color: Theme.colorTextOnAccent
                                font.pixelSize: Theme.fontSizeTiny
                                font.weight: Font.DemiBold
                                font.letterSpacing: 0.6
                            }
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Icon {
                        name: "x"
                        size: 16
                        opacity: root._busy ? 0.45 : 1

                        TapHandler {

                            gesturePolicy: TapHandler.ReleaseWithinBounds
                            enabled: !root._busy
                            onTapped: root._cancel()
                        }

                        HoverHandler {
                            enabled: !root._busy
                            cursorShape: Qt.PointingHandCursor
                        }
                    }
                }

                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: 1
                    color: Theme.colorDivider
                }
            }

            // -- Error banner, inside the dialog so it is seen while the modal is open
            Rectangle {
                visible: root._error !== null
                Layout.fillWidth: true
                Layout.leftMargin: 18
                Layout.rightMargin: 18
                Layout.topMargin: 14
                Layout.preferredHeight: errorRow.implicitHeight + 20
                radius: Theme.radiusMedium
                color: Theme.colorAccentRedSoft
                border.width: 1
                border.color: Theme.colorAccentRedLine

                RowLayout {
                    id: errorRow
                    anchors.fill: parent
                    anchors.margins: 10
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12
                    spacing: 10

                    Icon {
                        Layout.alignment: Qt.AlignTop
                        name: root._error && root._error.kind === SessionController.NoNetwork ? "wifi-off" : "circle-alert"
                        size: 15
                        color: Theme.colorAccentRed
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2

                        Text {
                            Layout.fillWidth: true
                            text: root._error ? root._errorTitle(root._error) : ""
                            color: Theme.colorTextPrimary
                            font.pixelSize: Theme.fontSizeMedium
                            font.weight: Font.DemiBold
                            wrapMode: Text.Wrap
                        }

                        Text {
                            Layout.fillWidth: true
                            text: root._error ? root._errorDetail(root._error) : ""
                            color: Theme.colorTextSecondary
                            font.pixelSize: Theme.fontSizeSmall
                            wrapMode: Text.Wrap
                        }
                    }
                }
            }

            // -- Body
            ColumnLayout {
                Layout.fillWidth: true
                Layout.margins: 18
                Layout.topMargin: 16
                spacing: 14
                enabled: !root._busy && !root._confirming
                opacity: enabled ? 1 : 0.5

                // Project — activity
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 6

                    FieldLabel {
                        text: "Project — Activity"
                        changed: root._projectChanged
                    }

                    ProjectActivityField {
                        id: projectField
                        Layout.fillWidth: true
                        Layout.preferredHeight: 36
                        sourceModel: root.sessionContext.activityModel
                        projectId: root._projectId
                        activityId: root._activityId
                        projectName: root._projectName
                        activityName: root._activityName
                        activityColor: root._activityColor
                        onCreateRequested: (kind, name) => creationDialog.openFor(kind, name, root._projectId)
                        onPairSelected: (projectId, activityId, projectName, activityName, activityColor) => {
                            root._projectId = projectId
                            root._activityId = activityId
                            root._projectName = projectName
                            root._activityName = activityName
                            root._activityColor = activityColor
                        }
                    }
                }

                // Description
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 6

                    FieldLabel {
                        text: "Description"
                        changed: root._descriptionChanged
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: Math.max(36, descriptionEdit.implicitHeight)
                        radius: Theme.radiusMedium
                        color: Theme.colorSurface
                        border.width: 1
                        border.color: descriptionEdit.activeFocus ? Theme.colorAccentGreenDim : "transparent"

                        Rectangle {
                            visible: descriptionEdit.activeFocus
                            anchors.fill: parent
                            anchors.margins: -3
                            radius: Theme.radiusMedium + 3
                            color: "transparent"
                            border.width: 2
                            border.color: Theme.colorAccentGreenSoft
                        }

                        TextEdit {
                            id: descriptionEdit
                            anchors.left: parent.left
                            anchors.right: parent.right
                            topPadding: 9
                            bottomPadding: 9
                            leftPadding: 10
                            rightPadding: 10
                            wrapMode: TextEdit.Wrap
                            color: Theme.colorTextPrimary
                            selectionColor: Theme.colorAccentGreenDim
                            font.pixelSize: Theme.fontSizeMedium
                            activeFocusOnTab: true
                            // Tab moves on instead of inserting a tab character
                            KeyNavigation.priority: KeyNavigation.BeforeItem
                            KeyNavigation.tab: startChip

                            Text {
                                visible: descriptionEdit.text === ""
                                x: descriptionEdit.leftPadding
                                y: descriptionEdit.topPadding
                                text: "No description"
                                color: Theme.colorTextDim
                                font.pixelSize: Theme.fontSizeMedium
                            }
                        }
                    }
                }

                // Time
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 6

                    FieldLabel {
                        text: "Time"
                        changed: root._timeChanged
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        TimeChip {
                            id: startChip
                            which: "Start"
                            value: root._begin
                            mode: root._punch ? "locked" : "normal"
                            onPickRequested: {
                                picker.editEnd = false
                                picker.title = "Edit Start Time"
                                picker.initialDateTime = root._begin
                                picker.open()
                            }
                        }

                        Icon {
                            name: "arrow-right"
                            size: 14
                        }

                        TimeChip {
                            id: endChip
                            which: "End"
                            value: root._end
                            mode: root.running ? "now" : (root._punch ? "locked" : (root._rangeValid ? "normal" : "error"))
                            onPickRequested: {
                                picker.editEnd = true
                                picker.title = "Edit End Time"
                                picker.initialDateTime = root._end
                                picker.open()
                            }
                        }

                        ColumnLayout {
                            Layout.preferredWidth: 84
                            spacing: 1

                            Text {
                                Layout.alignment: Qt.AlignRight
                                text: "Duration"
                                color: Theme.colorTextSecondary
                                font.pixelSize: Theme.fontSizeXSmall
                            }

                            Text {
                                Layout.alignment: Qt.AlignRight
                                text: !root._rangeValid ? "—" : root._formatDuration(root._begin, root.running ? root._now : root._end)
                                color: !root._rangeValid ? Theme.colorAccentRed : root.running ? Theme.colorAccentGreen : Theme.colorTextPrimary
                                font.pixelSize: Theme.fontSizeTitle
                                font.weight: Font.DemiBold
                                font.features: Theme.fontFeaturesTabular
                            }
                        }
                    }

                    RowLayout {
                        visible: !root._rangeValid || root.running || root._punch
                        Layout.topMargin: 2
                        spacing: 6

                        Icon {
                            name: root._rangeValid ? "lock" : "circle-alert"
                            size: root._rangeValid ? 12 : 13
                            color: root._rangeValid ? Theme.colorTextSecondary : Theme.colorAccentRed
                        }

                        Text {
                            text: !root._rangeValid ? "End must be after start (" + root._shortPoint(root._begin) + ")." : root._punch ? "Punch mode: start and end are managed by Kimai." : "End is set when you stop the timer."
                            color: root._rangeValid ? Theme.colorTextSecondary : Theme.colorAccentRed
                            font.pixelSize: Theme.fontSizeSmall
                        }
                    }
                }

                // Tags
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 6

                    FieldLabel {
                        text: "Tags"
                        changed: root._tagsChanged
                    }

                    TagsInput {
                        id: tagsInput
                        Layout.fillWidth: true
                        Layout.preferredHeight: tagsInput.implicitHeight
                        sessionContext: root.sessionContext
                    }
                }
            }

            // -- Footer: normal
            Item {
                visible: !root._confirming
                Layout.fillWidth: true
                Layout.preferredHeight: 58

                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    height: 1
                    color: Theme.colorDivider
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 18
                    anchors.rightMargin: 18
                    spacing: 8

                    AbstractButton {
                        id: deleteButton
                        enabled: !root._busy
                        opacity: enabled ? 1 : 0.45
                        focusPolicy: Qt.StrongFocus
                        padding: 4
                        onClicked: root._phase = "confirmDelete"

                        HoverHandler {
                            cursorShape: Qt.PointingHandCursor
                        }

                        contentItem: RowLayout {
                            spacing: 6

                            Icon {
                                name: "trash-2"
                                size: 13
                                color: deleteButton.hovered || deleteButton.visualFocus ? Theme.colorAccentRed : Theme.colorTextSecondary
                            }

                            Text {
                                text: "Delete entry"
                                color: deleteButton.hovered || deleteButton.visualFocus ? Theme.colorAccentRed : Theme.colorTextSecondary
                                font.pixelSize: Theme.fontSizeSmall
                            }
                        }

                        background: Item {}
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    RowLayout {
                        visible: root._dirty
                        Layout.rightMargin: 4
                        spacing: 6

                        Rectangle {
                            Layout.preferredWidth: 6
                            Layout.preferredHeight: 6
                            radius: 3
                            color: Theme.colorAccentGreen
                        }

                        Text {
                            text: "Unsaved changes"
                            color: Theme.colorTextSecondary
                            font.pixelSize: Theme.fontSizeSmall
                        }
                    }

                    DialogButton {
                        text: "Cancel"
                        enabled: !root._busy
                        onClicked: root._cancel()
                    }

                    DialogButton {
                        Layout.minimumWidth: 92
                        variant: "primary"
                        text: root._phase === "saving" && root._spinnerShown ? "Saving…" : (root._error && root._error.op === "save") ? "Retry" : "Save"
                        busy: root._phase === "saving"
                        spinning: root._spinnerShown
                        enabled: root._canSave || root._busy
                        onClicked: root._save()
                    }
                }
            }

            // -- Footer: delete confirmation
            Rectangle {
                visible: root._confirming
                Layout.fillWidth: true
                Layout.preferredHeight: 62
                color: Theme.colorAccentRedSoft
                bottomLeftRadius: Theme.radiusDialog
                bottomRightRadius: Theme.radiusDialog

                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    height: 1
                    color: Theme.colorAccentRedLine
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 18
                    anchors.rightMargin: 18
                    spacing: 12

                    Icon {
                        name: "trash-2"
                        size: 16
                        color: Theme.colorAccentRed
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2

                        Text {
                            text: "Delete this entry?"
                            color: Theme.colorTextPrimary
                            font.pixelSize: Theme.fontSizeMedium
                            font.weight: Font.DemiBold
                        }

                        Text {
                            Layout.fillWidth: true
                            text: (root.running ? "The timer will stop. " : root._origDuration + " on " + root._origProjectName + ". ") + "This can't be undone."
                            color: Theme.colorTextSecondary
                            font.pixelSize: Theme.fontSizeSmall
                            elide: Text.ElideRight
                        }
                    }

                    DialogButton {
                        id: keepButton
                        text: "Keep entry"
                        enabled: !root._busy
                        onClicked: root._phase = "idle"
                    }

                    DialogButton {
                        variant: "danger"
                        text: root._phase === "deleting" && root._spinnerShown ? "Deleting…" : "Delete"
                        busy: root._phase === "deleting"
                        spinning: root._spinnerShown
                        enabled: root._phase === "confirmDelete"
                        onClicked: root._delete()
                    }
                }
            }
        }
    }
}
