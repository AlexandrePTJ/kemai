// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import Qt.labs.platform as Platform

import Kemai

// Native tray icon: timer state at a glance, window toggle, stop and quit
Platform.SystemTrayIcon {
    id: root

    // Set by Main.qml (not required: created by an Instantiator)
    property Window window: null
    // Null on the login view: "Not connected"
    property SessionContext session: null

    signal quitRequested

    readonly property bool _running: root.session !== null && root.session.hasActiveTimesheet
    // macOS menu bar: monochrome template images, recolored by the system
    readonly property bool _template: Qt.platform.os === "osx" || Qt.platform.os === "macos"
    // StatusNotifierItem hosts show the tooltip as one line
    readonly property bool _singleLineTooltip: Qt.platform.os === "linux"

    // Minute precision: the tooltip is not rebuilt every second
    property date _now: new Date()
    // When the window last lost the focus: clicking the tray takes it away before `activated` arrives
    property double _deactivatedAt: 0
    // Whether the window was in front when the menu opened ("Show"/"Hide")
    property bool _inFrontAtMenu: false

    visible: root.available
    icon.source: "qrc:/assets/images/tray/kemai-tray-" + (root._running ? "running" : "idle") + (root._template ? "-template" : "") + ".svg"
    icon.mask: root._template
    tooltip: root._tooltip()

    onActivated: reason => {
        if (reason === Platform.SystemTrayIcon.Trigger)
            root.toggleWindow()
    }

    function toggleWindow() {
        if (root._windowInFront())
            root.window.hide()
        else
            root.showWindow()
    }

    function showWindow() {
        if (root.window.visibility === Window.Minimized)
            root.window.visibility = root._restoreVisibility
        else
            root.window.show()
        root.window.raise()
        root.window.requestActivate()
    }

    function _windowInFront() {
        const w = root.window
        if (!w.visible || w.visibility === Window.Minimized)
            return false
        return w.active || Date.now() - root._deactivatedAt < 500
    }

    // Shown again maximized when it was minimized from maximized
    property int _restoreVisibility: Window.Windowed

    // SystemTrayIcon has no default property: helpers are held by properties
    readonly property Connections _windowWatch: Connections {
        target: root.window
        function onActiveChanged() {
            if (!root.window.active)
                root._deactivatedAt = Date.now()
        }
        function onVisibilityChanged() {
            const v = root.window.visibility
            if (v === Window.Windowed || v === Window.Maximized || v === Window.FullScreen)
                root._restoreVisibility = v
        }
    }

    readonly property Timer _minuteTick: Timer {
        interval: 60000
        repeat: true
        running: root._running
        onTriggered: root._now = new Date()
    }
    readonly property Connections _sessionWatch: Connections {
        target: root.session
        function onActiveTimesheetChanged() {
            root._now = new Date()
        }
    }

    function _elapsed() {
        const minutes = Math.max(0, Math.floor((root._now - root.session.activeTimesheetBegin) / 60000))
        if (minutes < 60)
            return minutes + "m"
        const rest = minutes % 60
        return Math.floor(minutes / 60) + "h " + (rest < 10 ? "0" : "") + rest + "m"
    }

    function _entry() {
        return root.session.activeProjectName + " — " + root.session.activeActivityName
    }

    function _tooltip() {
        if (root.session === null)
            return qsTr("Kemai · Not connected")
        if (!root._running)
            return qsTr("Kemai · No timer running")
        if (root._singleLineTooltip)
            return qsTr("Kemai · %1 · %2").arg(root._entry()).arg(root._elapsed())
        return qsTr("Kemai · Running for %1").arg(root._elapsed()) + "\n" + root._entry()
    }

    // "Project — Activity" within about 40 characters: the activity is cut first
    function _stopTarget() {
        const max = 40
        const project = root.session.activeProjectName
        const full = root._entry()
        if (full.length <= max)
            return full
        const room = max - project.length - 4
        if (room >= 1)
            return project + " — " + root.session.activeActivityName.slice(0, room).trim() + "…"
        return project.slice(0, max - 1).trim() + "…"
    }

    menu: Platform.Menu {
        onAboutToShow: root._inFrontAtMenu = root._windowInFront()

        Platform.MenuItem {
            text: root._inFrontAtMenu ? qsTr("Hide Kemai") : qsTr("Show Kemai")
            onTriggered: {
                if (root._inFrontAtMenu)
                    root.window.hide()
                else
                    root.showWindow()
            }
        }
        Platform.MenuSeparator {}
        // Never hidden: the menu does not shift under the cursor
        Platform.MenuItem {
            text: root._running ? qsTr("Stop “%1”").arg(root._stopTarget()) : qsTr("Stop timer")
            enabled: root._running
            onTriggered: {
                // A failure shows in the window's banner: make sure it can be seen
                if (!root.window.visible || root.window.visibility === Window.Minimized)
                    root.showWindow()
                root.session.stopActiveTimeSheet()
            }
        }
        Platform.MenuSeparator {}
        Platform.MenuItem {
            text: qsTr("Quit Kemai")
            onTriggered: root.quitRequested()
        }
    }
}
