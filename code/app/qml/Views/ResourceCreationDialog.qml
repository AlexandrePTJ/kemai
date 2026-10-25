// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kemai

// Creates the activity or project the user searched for (and its customer when needed), then hands the new
// project — activity pair back to the picker the search came from. Steps run one after the other with the id
// of the previous one; after a partial failure, Retry runs only the remaining steps.
Popup {
    id: root

    required property SessionContext sessionContext

    // Created (or existing, "Use it") pair: {projectId, activityId, projectName, activityName, activityColor, label, isGlobal}
    signal pairChosen(var pair)
    // For the banner of the view behind: NotificationKind.ResourceCreated detail
    signal created(string detail)

    // -- Form state
    property string _form: "activity" // activity | project

    // Activity form
    property bool _activityGlobal: false
    property var _activityProjectId: null

    // Project form
    property bool _newCustomer: false
    property var _customerId: null
    property string _country: ""
    property string _currency: ""
    property string _timezone: ""
    property int _activityMode: 0 // 0: existing global activity, 1: new activity
    property var _globalActivityId: null

    // Choices, read when the dialog opens
    property var _projects: []
    property var _customers: []
    property var _globals: []
    property var _countries: []
    property var _currencies: []
    property var _timezones: []

    // -- Run state
    property string _phase: "idle" // idle | saving | error | finishing
    property bool _spinnerShown: false
    // [{kind: "customer" | "project" | "activity", label, status: "todo" | "run" | "done" | "fail", id}]
    property var _steps: []
    // null or {resource, kind (NotificationKind), detail, field}
    property var _error: null
    // Pair to select once the resources are reloaded
    property var _pendingPair: null

    // Field errors ("already exists" has a "Use it" link)
    property string _activityNameError: ""
    property string _projectNameError: ""
    property string _customerNameError: ""
    property var _useExisting: null // function run by the "Use it" link

    readonly property bool _busy: root._phase === "saving" || root._phase === "finishing"

    function openFor(kind, query, defaultProjectId) {
        const name = query.trim()
        root._loadChoices()
        root._resetRun()

        root._form = kind
        activityNameField.text = kind === "activity" ? name : ""
        projectNameField.text = kind === "project" ? name : ""
        root._activityGlobal = false
        root._activityProjectId = root._projectMatching(name, defaultProjectId)
        root._newCustomer = false
        root._customerId = null
        root._activityMode = 0
        root._globalActivityId = root._globals.length > 0 ? root._globals[0].key : null
        newActivityField.text = ""
        customerNameField.text = ""
        root._country = reference.systemCountry()
        root._currency = reference.systemCurrency()
        root._timezone = root.sessionContext.userTimezone

        root.open()
    }

    function _loadChoices() {
        root._projects = root.sessionContext.projectChoices().map(p => ({
                    key: p.id,
                    label: p.name,
                    meta: p.customerName,
                    color: p.color
                }))
        root._customers = root.sessionContext.customerChoices().map(c => ({
                    key: c.id,
                    label: c.name,
                    meta: c.projectCount === 1 ? "1 project" : c.projectCount + " projects"
                }))
        root._globals = root.sessionContext.globalActivityChoices().map(a => ({
                    key: a.id,
                    label: a.name,
                    color: a.color,
                    meta: "global"
                }))
        if (root._countries.length === 0) {
            root._countries = reference.countries().map(c => ({
                        key: c.code,
                        label: c.name,
                        code: c.code
                    }))
            root._currencies = reference.currencies().map(c => ({
                        key: c.code,
                        label: c.name,
                        meta: c.code
                    }))
            root._timezones = reference.timezones().map(t => ({
                        key: t,
                        label: t
                    }))
        }
    }

    // A project whose name holds part of the search, else the one of the last used entry
    function _projectMatching(name, fallbackId) {
        const words = name.toLowerCase().split(/\s+/).filter(w => w.length >= 3)
        for (const project of root._projects) {
            const label = project.label.toLowerCase()
            if (words.some(w => label.includes(w)))
                return project.key
        }
        // The last entry's project may be hidden (not offered): then the first one
        if (fallbackId > 0 && root._item(root._projects, fallbackId) !== null)
            return fallbackId
        return root._projects.length > 0 ? root._projects[0].key : null
    }

    function _resetRun() {
        root._phase = "idle"
        root._spinnerShown = false
        root._steps = []
        root._error = null
        root._pendingPair = null
        root._activityNameError = ""
        root._projectNameError = ""
        root._customerNameError = ""
        root._useExisting = null
    }

    function _item(list, key) {
        for (const item of list) {
            if (item.key === key)
                return item
        }
        return null
    }

    function _stepDone(kind) {
        return root._steps.some(s => s.kind === kind && s.status === "done")
    }

    function _stepId(kind) {
        const step = root._steps.find(s => s.kind === kind)
        return step ? step.id : 0
    }

    // What the form will create, for the header: "customer, project, activity"
    readonly property string _createdKinds: {
        if (root._form === "activity")
            return "activity"
        const kinds = []
        if (root._newCustomer)
            kinds.push("customer")
        kinds.push("project")
        if (root._activityMode === 1)
            kinds.push("activity")
        return kinds.join(", ")
    }

    readonly property bool _valid: {
        if (root._form === "activity")
            return activityNameField.text.trim() !== "" && root._item(root._projects, root._activityProjectId) !== null
        if (projectNameField.text.trim() === "")
            return false
        if (root._newCustomer) {
            if (customerNameField.text.trim() === "" || root._country === "" || root._currency === "" || root._timezone === "")
                return false
        } else if (root._customerId === null) {
            return false
        }
        return root._activityMode === 0 ? root._globalActivityId !== null : newActivityField.text.trim() !== ""
    }

    // Pair the form will select, for the preview
    readonly property var _preview: {
        if (root._form === "activity") {
            const project = root._item(root._projects, root._activityProjectId)
            return project ? {
                project: project.label,
                activity: activityNameField.text.trim(),
                color: project.color,
                global: root._activityGlobal
            } : null
        }
        const globalActivity = root._item(root._globals, root._globalActivityId)
        return {
            project: projectNameField.text.trim(),
            activity: root._activityMode === 1 ? newActivityField.text.trim() : (globalActivity ? globalActivity.label : ""),
            color: root._activityMode === 0 && globalActivity ? globalActivity.color : Theme.colorAccentGreen,
            global: root._activityMode === 0
        }
    }

    function _switchToProjectForm() {
        // Values entered so far carry over: the activity becomes the project's new activity
        newActivityField.text = activityNameField.text.trim()
        root._activityMode = 1
        root._form = "project"
        root._resetRun()
        projectNameField.forceActiveFocus()
    }

    function _useCustomer(id) {
        root._newCustomer = false
        root._customerId = id
        root._customerNameError = ""
    }

    function _submit() {
        if (root._busy || !root._valid)
            return

        // First run: check names against what Kimai already has (it accepts duplicates)
        if (root._steps.length === 0 && !root._checkExisting())
            return
        if (root._steps.length === 0) {
            if (root._form === "activity") {
                root._steps = [
                    {
                        kind: "activity",
                        label: "Activity",
                        status: "todo",
                        id: 0
                    }
                ]
            } else {
                const steps = []
                if (root._newCustomer)
                    steps.push({
                        kind: "customer",
                        label: "Customer",
                        status: "todo",
                        id: 0
                    })
                steps.push({
                    kind: "project",
                    label: "Project",
                    status: "todo",
                    id: 0
                })
                if (root._activityMode === 1)
                    steps.push({
                        kind: "activity",
                        label: "Activity",
                        status: "todo",
                        id: 0
                    })
                root._steps = steps
            }
        }

        root._error = null
        root._phase = "saving"
        root._spinnerShown = false
        spinnerDelay.restart()
        root._runNextStep()
    }

    // Returns false when a name already exists (the field shows it, with "Use it")
    function _checkExisting() {
        root._activityNameError = ""
        root._projectNameError = ""
        root._customerNameError = ""

        if (root._form === "activity") {
            const name = activityNameField.text.trim()
            const scopeProject = root._activityGlobal ? 0 : root._activityProjectId
            const existing = root.sessionContext.findActivity(name, scopeProject)
            if (existing.id !== undefined) {
                const project = root._item(root._projects, root._activityProjectId)
                root._activityNameError = "“" + existing.name + "” already exists" + (root._activityGlobal ? " as a global activity." : " in " + project.label + ".")
                root._useExisting = () => root._choosePair(root._activityProjectId, existing.id, project.label, existing.name)
                return false
            }
            return true
        }

        if (root._newCustomer) {
            const customer = root.sessionContext.findCustomer(customerNameField.text.trim())
            if (customer.id !== undefined) {
                root._customerNameError = "“" + customer.name + "” already exists."
                root._useExisting = () => root._useCustomer(customer.id)
                return false
            }
        } else {
            const project = root.sessionContext.findProject(projectNameField.text.trim(), root._customerId)
            if (project.id !== undefined) {
                root._projectNameError = "“" + project.name + "” already exists in " + project.customerName + "."
                root._useExisting = () => {
                    // Track on the existing project instead: the activity form, project chosen
                    activityNameField.text = root._activityMode === 1 ? newActivityField.text.trim() : ""
                    root._activityProjectId = project.id
                    root._activityGlobal = false
                    root._form = "activity"
                    root._resetRun()
                    activityNameField.forceActiveFocus()
                }
                return false
            }
        }
        return true
    }

    function _setStep(kind, status, id) {
        root._steps = root._steps.map(s => s.kind === kind ? {
                kind: s.kind,
                label: s.label,
                status: status,
                id: id !== undefined ? id : s.id
            } : s)
    }

    function _runNextStep() {
        const step = root._steps.find(s => s.status !== "done")
        if (!step) {
            root._finish()
            return
        }
        root._setStep(step.kind, "run")

        if (step.kind === "customer") {
            root.sessionContext.createCustomer(customerNameField.text.trim(), root._country, root._currency, root._timezone)
        } else if (step.kind === "project") {
            const customerId = root._newCustomer ? root._stepId("customer") : root._customerId
            root.sessionContext.createProject(projectNameField.text.trim(), customerId)
        } else {
            const name = root._form === "activity" ? activityNameField.text.trim() : newActivityField.text.trim()
            const projectId = root._form === "activity" ? (root._activityGlobal ? 0 : root._activityProjectId) : root._stepId("project")
            root.sessionContext.createActivity(name, projectId)
        }
    }

    function _finish() {
        let projectId, activityId, projectName, activityName, detail
        if (root._form === "activity") {
            const project = root._item(root._projects, root._activityProjectId)
            projectId = root._activityProjectId
            activityId = root._stepId("activity")
            projectName = project.label
            activityName = activityNameField.text.trim()
            detail = "activity|" + activityName + "|" + (root._activityGlobal ? "" : projectName)
        } else {
            const globalActivity = root._item(root._globals, root._globalActivityId)
            projectId = root._stepId("project")
            activityId = root._activityMode === 1 ? root._stepId("activity") : root._globalActivityId
            projectName = projectNameField.text.trim()
            activityName = root._activityMode === 1 ? newActivityField.text.trim() : globalActivity.label
            detail = "project|" + projectName + "|"
        }
        root._pendingPair = {
            projectId: projectId,
            activityId: activityId,
            projectName: projectName,
            activityName: activityName,
            detail: detail
        }
        root._phase = "finishing"
        // Make the new items searchable, then hand the pair over (as the reloaded cache shows it)
        root.sessionContext.reloadResources()
        if (!root.sessionContext.refreshing)
            root._handOver()
    }

    function _handOver() {
        const pending = root._pendingPair
        root.created(pending.detail)
        root._choosePair(pending.projectId, pending.activityId, pending.projectName, pending.activityName)
    }

    function _choosePair(projectId, activityId, projectName, activityName) {
        const cached = root.sessionContext.pair(projectId, activityId)
        root.pairChosen(cached.projectId !== undefined ? cached : {
            projectId: projectId,
            activityId: activityId,
            projectName: projectName,
            activityName: activityName,
            activityColor: Theme.colorAccentGreen,
            label: projectName + " - " + activityName,
            isGlobal: false
        })
        root._phase = "idle"
        root.close()
    }

    function _onFailed(resourceKind, errorKind, detail, field) {
        const kind = resourceKind === SessionController.Customer ? "customer" : resourceKind === SessionController.Project ? "project" : "activity"
        root._setStep(kind, "fail")
        root._error = {
            resource: kind,
            kind: errorKind,
            detail: detail,
            field: field
        }
        root._phase = "error"
    }

    // Banner of a failure: {icon, title, detail}
    readonly property var _banner: {
        if (root._error === null)
            return null
        const e = root._error
        const plural = {
            customer: "customers",
            project: "projects",
            activity: "activities"
        }[e.resource]
        const done = root._steps.filter(s => s.status === "done")
        if (done.length > 0) {
            const created = done.map(s => s.label).join(", ")
            const remaining = root._steps.filter(s => s.status !== "done").map(s => s.label.toLowerCase()).join(" and ")
            return {
                icon: e.kind === SessionController.NoNetwork ? "wifi-off" : "circle-alert",
                title: created + " created, " + e.resource + " not created",
                detail: (e.kind === SessionController.InvalidValue && e.detail !== "" ? e.detail + " " : "") + "What was created stays in Kimai — Retry creates only the " + remaining + "."
            }
        }
        switch (e.kind) {
        case SessionController.Forbidden:
            return {
                icon: "lock",
                title: "Kimai refused — you can't create " + plural,
                detail: "Your account lacks this permission. Pick an existing " + e.resource + " or ask a Kimai admin."
            }
        case SessionController.InvalidValue:
            return {
                icon: "circle-alert",
                title: "Kimai refused — invalid value",
                detail: (e.field !== "" ? e.field.charAt(0).toUpperCase() + e.field.slice(1) + ": " : "") + e.detail
            }
        case SessionController.NoNetwork:
            return {
                icon: "wifi-off",
                title: "Couldn't create — no connection",
                detail: "Nothing was created. Your values are kept; check the network and retry."
            }
        case SessionController.ServerError:
            return {
                icon: "circle-alert",
                title: "Couldn't create — server error",
                detail: "Kimai returned an error. Your values are kept; retry in a moment."
            }
        default:
            return {
                icon: "circle-alert",
                title: "Couldn't create — unexpected error",
                detail: e.detail !== "" ? e.detail : "Your values are kept; retry in a moment."
            }
        }
    }

    // A refused value on a name: highlight that name's field
    function _nameRefused(kind) {
        return root._error !== null && root._error.kind === SessionController.InvalidValue && root._error.resource === kind && root._error.field === "name"
    }

    ReferenceData {
        id: reference
    }

    // Only while this dialog has a request in flight (another one may be creating too; `opened` is
    // only true once the open transition ends, a quick Enter would miss its answer)
    Connections {
        target: root.sessionContext
        enabled: root._busy
        function onResourceCreated(kind, id) {
            const step = root._steps.find(s => s.status === "run")
            if (!step)
                return
            root._setStep(step.kind, "done", id)
            root._runNextStep()
        }
        function onResourceCreateFailed(kind, error, detail, field) {
            root._onFailed(kind, error, detail, field)
        }
        function onRefreshingChanged() {
            if (root._phase === "finishing" && !root.sessionContext.refreshing)
                root._handOver()
        }
    }

    Timer {
        id: spinnerDelay
        interval: 150
        onTriggered: root._spinnerShown = true
    }

    parent: Overlay.overlay
    modal: true
    focus: true
    closePolicy: root._busy ? Popup.NoAutoClose : Popup.CloseOnEscape
    width: Math.min(root._form === "activity" ? 540 : 560, parent ? parent.width - 40 : 560)
    height: Math.min(dialogColumn.implicitHeight, parent ? parent.height - 40 : 560)
    x: parent ? (parent.width - width) / 2 : 0
    y: parent ? (parent.height - height) / 2 : 0
    padding: 0

    onOpened: (root._form === "activity" ? activityNameField : projectNameField).forceActiveFocus()

    Overlay.modal: Rectangle {
        color: Theme.colorBackdrop
    }

    background: Rectangle {
        color: Theme.colorBackground
        border.color: Theme.colorBorderSubtle
        border.width: 1
        radius: Theme.radiusDialog
    }

    contentItem: ColumnLayout {
        id: dialogColumn
        spacing: 0

        // -- Header
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 18
            Layout.rightMargin: 18
            Layout.topMargin: 13
            Layout.bottomMargin: 13
            spacing: 10

            Rectangle {
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                radius: Theme.radiusMedium
                color: Theme.colorAccentGreenSoft
                border.width: 1
                border.color: Theme.colorAccentGreenDim

                Icon {
                    anchors.centerIn: parent
                    name: "plus"
                    size: 12
                    color: Theme.colorAccentGreen
                }
            }

            Text {
                text: root._form === "activity" ? "New activity" : "New project"
                color: Theme.colorTextPrimary
                font.pixelSize: Theme.fontSizeNormal
                font.weight: Font.DemiBold
            }

            Text {
                Layout.fillWidth: true
                text: "in Kimai · " + root._createdKinds
                color: Theme.colorTextSecondary
                font.pixelSize: Theme.fontSizeSmall
                elide: Text.ElideRight
            }

            Icon {
                name: "x"
                size: 16
                color: Theme.colorTextSecondary
                opacity: root._busy ? 0.45 : 1

                TapHandler {

                    gesturePolicy: TapHandler.ReleaseWithinBounds
                    enabled: !root._busy
                    onTapped: root.close()
                }

                HoverHandler {
                    enabled: !root._busy
                    cursorShape: Qt.PointingHandCursor
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: Theme.colorDivider
        }

        // -- Failure banner
        Rectangle {
            visible: root._banner !== null
            Layout.fillWidth: true
            Layout.leftMargin: 18
            Layout.rightMargin: 18
            Layout.topMargin: 14
            Layout.preferredHeight: bannerRow.implicitHeight + 20
            radius: Theme.radiusMedium
            color: Theme.colorAccentRedSoft
            border.width: 1
            border.color: Theme.colorAccentRedLine

            RowLayout {
                id: bannerRow
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 10

                Icon {
                    Layout.alignment: Qt.AlignTop
                    Layout.topMargin: 1
                    name: root._banner !== null ? root._banner.icon : "circle-alert"
                    size: 15
                    color: Theme.colorAccentRed
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    Text {
                        Layout.fillWidth: true
                        text: root._banner !== null ? root._banner.title : ""
                        color: Theme.colorTextPrimary
                        font.pixelSize: Theme.fontSizeMedium
                        font.weight: Font.DemiBold
                        wrapMode: Text.Wrap
                    }

                    Text {
                        Layout.fillWidth: true
                        text: root._banner !== null ? root._banner.detail : ""
                        color: Theme.colorTextSecondary
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.Wrap
                    }
                }
            }
        }

        // -- Body (scrolls when the window is too short; header and footer stay)
        Flickable {
            id: bodyFlick
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: bodyColumn.implicitHeight + 30
            clip: true
            contentWidth: width
            contentHeight: bodyColumn.implicitHeight + 30
            boundsBehavior: Flickable.StopAtBounds
            interactive: contentHeight > height

            WheelScroller {
                flickable: bodyFlick
            }

            ColumnLayout {
                id: bodyColumn
                x: 18
                y: 14
                width: bodyFlick.width - 36
                spacing: 12
                enabled: !root._busy
                opacity: root._busy ? 0.5 : 1

                // ---- Activity form
                FormField {
                    visible: root._form === "activity"
                    Layout.fillWidth: true
                    label: "Activity name"
                    note: "from your search"
                    error: root._activityNameError
                    linkText: root._activityNameError !== "" ? "Use it" : ""
                    onLinkActivated: root._useExisting()

                    FormTextField {
                        id: activityNameField
                        Layout.fillWidth: true
                        hasError: root._activityNameError !== "" || root._nameRefused("activity")
                        enabled: !root._stepDone("activity")
                        onTextChanged: root._activityNameError = ""
                        onAccepted: root._submit()
                    }
                }

                FormField {
                    visible: root._form === "activity"
                    Layout.fillWidth: true
                    label: "Belongs to"

                    SegmentedControl {
                        Layout.fillWidth: true
                        options: ["One project", "Global — any project"]
                        currentIndex: root._activityGlobal ? 1 : 0
                        onActivated: index => {
                            root._activityGlobal = index === 1
                            root._activityNameError = ""
                        }
                    }
                }

                FormField {
                    visible: root._form === "activity"
                    Layout.fillWidth: true
                    label: root._activityGlobal ? "Track it on project" : "Project"
                    note: root._activityGlobal ? "needed for this time entry only" : ""

                    SearchSelect {
                        Layout.fillWidth: true
                        items: root._projects
                        currentKey: root._activityProjectId
                        placeholder: "Choose a project"
                        createLabel: "New project… <font color=\"" + Theme.colorTextSecondary + "\">(opens project form)</font>"
                        createAlways: true
                        onSelected: item => {
                            root._activityProjectId = item.key
                            root._activityNameError = ""
                        }
                        onCreateRequested: query => {
                            if (query !== "")
                                projectNameField.text = query
                            root._switchToProjectForm()
                        }
                    }
                }

                // ---- Project form
                FormField {
                    visible: root._form === "project"
                    Layout.fillWidth: true
                    label: "Project name"
                    note: "from your search"
                    error: root._projectNameError
                    linkText: root._projectNameError !== "" ? "Use it" : ""
                    onLinkActivated: root._useExisting()

                    FormTextField {
                        id: projectNameField
                        Layout.fillWidth: true
                        hasError: root._projectNameError !== "" || root._nameRefused("project")
                        enabled: !root._stepDone("project")
                        onTextChanged: root._projectNameError = ""
                        onAccepted: root._submit()
                    }
                }

                FormField {
                    visible: root._form === "project" && !root._newCustomer
                    Layout.fillWidth: true
                    label: "Customer"

                    SearchSelect {
                        Layout.fillWidth: true
                        items: root._customers
                        currentKey: root._customerId
                        placeholder: "Choose a customer"
                        createLabel: "New customer {query}"
                        createOnlyWithoutMatch: true
                        onSelected: item => {
                            root._customerId = item.key
                            root._projectNameError = ""
                        }
                        onCreateRequested: query => {
                            customerNameField.text = query
                            root._newCustomer = true
                            root._projectNameError = ""
                            customerNameField.forceActiveFocus()
                        }
                    }
                }

                // New customer card
                Rectangle {
                    visible: root._form === "project" && root._newCustomer
                    Layout.fillWidth: true
                    Layout.preferredHeight: customerCard.implicitHeight + 24
                    radius: Theme.radiusMedium
                    color: Theme.colorSurfaceDark
                    border.width: 1
                    border.color: Theme.colorBorderSubtle

                    ColumnLayout {
                        id: customerCard
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 12
                        spacing: 10
                        // Created already (partial failure): never created twice
                        enabled: !root._stepDone("customer")
                        opacity: enabled ? 1 : 0.6

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Icon {
                                name: "plus"
                                size: 12
                                color: Theme.colorAccentGreen
                            }

                            Text {
                                Layout.fillWidth: true
                                text: "NEW CUSTOMER"
                                color: Theme.colorTextPrimary
                                font.pixelSize: Theme.fontSizeXSmall
                                font.weight: Font.DemiBold
                                font.letterSpacing: 1.3
                            }

                            RowLayout {
                                visible: !root._stepDone("customer")
                                spacing: 5

                                Icon {
                                    name: "x"
                                    size: 11
                                    color: Theme.colorTextSecondary
                                }

                                Text {
                                    text: "Pick existing instead"
                                    color: Theme.colorTextSecondary
                                    font.pixelSize: Theme.fontSizeSmall

                                    TapHandler {

                                        gesturePolicy: TapHandler.ReleaseWithinBounds
                                        onTapped: {
                                            root._newCustomer = false
                                            root._customerNameError = ""
                                        }
                                    }

                                    HoverHandler {
                                        cursorShape: Qt.PointingHandCursor
                                    }
                                }
                            }
                        }

                        FormField {
                            Layout.fillWidth: true
                            label: "Customer name"
                            error: root._customerNameError
                            linkText: root._customerNameError !== "" ? "Use it" : ""
                            onLinkActivated: root._useExisting()

                            FormTextField {
                                id: customerNameField
                                Layout.fillWidth: true
                                hasError: root._customerNameError !== "" || root._nameRefused("customer")
                                onTextChanged: root._customerNameError = ""
                                onAccepted: root._submit()
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            FormField {
                                Layout.fillWidth: true
                                Layout.preferredWidth: 100
                                label: "Country"
                                note: "system"

                                SearchSelect {
                                    Layout.fillWidth: true
                                    items: root._countries
                                    currentKey: root._country
                                    placeholder: "Choose"
                                    hasError: root._error !== null && root._error.field === "country"
                                    onSelected: item => root._country = item.key
                                }
                            }

                            FormField {
                                Layout.fillWidth: true
                                Layout.preferredWidth: 100
                                label: "Currency"
                                note: "system"

                                SearchSelect {
                                    Layout.fillWidth: true
                                    items: root._currencies
                                    currentKey: root._currency
                                    placeholder: "Choose"
                                    hasError: root._error !== null && root._error.field === "currency"
                                    onSelected: item => root._currency = item.key
                                }
                            }

                            FormField {
                                Layout.fillWidth: true
                                Layout.preferredWidth: 125
                                label: "Timezone"
                                note: "Kimai"

                                SearchSelect {
                                    Layout.fillWidth: true
                                    items: root._timezones
                                    currentKey: root._timezone
                                    placeholder: "Choose"
                                    hasError: root._error !== null && root._error.field === "timezone"
                                    onSelected: item => root._timezone = item.key
                                }
                            }
                        }
                    }
                }

                FormField {
                    visible: root._form === "project"
                    Layout.fillWidth: true
                    label: "Activity to track"

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        SegmentedControl {
                            Layout.fillWidth: true
                            Layout.preferredWidth: 1
                            options: ["Global activity", "New activity"]
                            currentIndex: root._activityMode
                            onActivated: index => root._activityMode = index
                        }

                        SearchSelect {
                            visible: root._activityMode === 0
                            Layout.fillWidth: true
                            Layout.preferredWidth: 1
                            items: root._globals
                            currentKey: root._globalActivityId
                            placeholder: root._globals.length > 0 ? "Choose an activity" : "No global activity"
                            onSelected: item => root._globalActivityId = item.key
                        }

                        FormTextField {
                            id: newActivityField
                            visible: root._activityMode === 1
                            Layout.fillWidth: true
                            Layout.preferredWidth: 1
                            hint: "Activity name"
                            hasError: root._nameRefused("activity")
                            onAccepted: root._submit()
                        }
                    }
                }

                // ---- Preview of the pair that will be selected
                Rectangle {
                    visible: root._preview !== null && !(root._form === "project" && root._newCustomer)
                    Layout.fillWidth: true
                    Layout.preferredHeight: 38
                    radius: Theme.radiusMedium
                    color: Theme.colorBackgroundDark

                    DashedBorder {
                        anchors.fill: parent
                        radius: Theme.radiusMedium
                        color: Theme.colorBorderSubtle
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12
                        spacing: 7

                        Text {
                            Layout.rightMargin: 3
                            text: "Will select"
                            color: Theme.colorTextSecondary
                            font.pixelSize: Theme.fontSizeSmall
                        }

                        Rectangle {
                            Layout.preferredWidth: 9
                            Layout.preferredHeight: 9
                            radius: 2
                            color: root._preview !== null ? root._preview.color : "transparent"
                        }

                        Text {
                            text: root._preview !== null ? root._preview.project : ""
                            color: Theme.colorTextPrimary
                            font.pixelSize: Theme.fontSizeSmall
                            font.weight: Font.DemiBold
                        }

                        Text {
                            text: "—"
                            color: Theme.colorTextDim
                            font.pixelSize: Theme.fontSizeSmall
                        }

                        Text {
                            Layout.fillWidth: !(root._preview !== null && root._preview.global)
                            text: root._preview !== null ? root._preview.activity : ""
                            color: Theme.colorTextSecondary
                            font.pixelSize: Theme.fontSizeSmall
                            elide: Text.ElideRight
                        }

                        Rectangle {
                            visible: root._preview !== null && root._preview.global
                            Layout.preferredWidth: globalBadge.implicitWidth + 10
                            Layout.preferredHeight: globalBadge.implicitHeight + 2
                            radius: 3
                            color: "transparent"
                            border.width: 1
                            border.color: Theme.colorBorderSubtle

                            Text {
                                id: globalBadge
                                anchors.centerIn: parent
                                text: "global"
                                color: Theme.colorTextSecondary
                                font.pixelSize: Theme.fontSizeTiny
                            }
                        }

                        Item {
                            visible: root._preview !== null && root._preview.global
                            Layout.fillWidth: true
                        }
                    }
                }
            }
        }

        // -- Footer
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: Theme.colorDivider
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 18
            Layout.rightMargin: 18
            Layout.topMargin: 12
            Layout.bottomMargin: 12
            spacing: 8

            Text {
                visible: root._steps.length <= 1
                Layout.fillWidth: true
                text: "Back to tracking with the new pair selected"
                color: Theme.colorTextSecondary
                font.pixelSize: Theme.fontSizeSmall
                elide: Text.ElideRight
            }

            // Progress of a creation in several steps
            RowLayout {
                visible: root._steps.length > 1
                Layout.fillWidth: true
                spacing: 12

                Repeater {
                    model: root._steps.length > 1 ? root._steps : []

                    delegate: RowLayout {
                        id: stepItem

                        required property var modelData

                        spacing: 5

                        Icon {
                            visible: stepItem.modelData.status === "done" || stepItem.modelData.status === "fail"
                            name: stepItem.modelData.status === "done" ? "check" : "x"
                            size: 12
                            color: stepItem.modelData.status === "done" ? Theme.colorAccentGreen : Theme.colorAccentRed
                        }

                        Spinner {
                            visible: stepItem.modelData.status === "run"
                            size: 12
                            color: Theme.colorAccentGreen
                            trackColor: Theme.colorBorderSubtle
                        }

                        Rectangle {
                            visible: stepItem.modelData.status === "todo"
                            Layout.preferredWidth: 10
                            Layout.preferredHeight: 10
                            radius: 5
                            color: "transparent"
                            border.width: 1.5
                            border.color: Theme.colorTextDim
                        }

                        Text {
                            text: stepItem.modelData.label
                            color: stepItem.modelData.status === "fail" ? Theme.colorAccentRed : stepItem.modelData.status === "done" ? Theme.colorTextPrimary : Theme.colorTextSecondary
                            font.pixelSize: Theme.fontSizeSmall
                        }
                    }
                }

                Item {
                    Layout.fillWidth: true
                }
            }

            DialogButton {
                text: "Cancel"
                enabled: !root._busy
                onClicked: root.close()
            }

            DialogButton {
                Layout.minimumWidth: 120
                variant: "primary"
                text: root._busy && root._spinnerShown ? "Creating…" : root._phase === "error" ? "Retry" : root._form === "activity" ? "Create activity" : "Create & select"
                busy: root._busy
                spinning: root._spinnerShown
                enabled: root._valid || root._busy
                onClicked: root._submit()
            }
        }
    }
}
