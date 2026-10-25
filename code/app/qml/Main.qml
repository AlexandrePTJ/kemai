// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import Kemai

ApplicationWindow {
    id: main

    // Set by main(): the log of this run, for the log journal
    required property LogJournal logJournal

    // Default size; shown by appContext with the saved geometry once completed
    width: 800
    height: 600
    title: appContext.windowTitle

    // Geometry of the window when not maximized, saved on close
    property rect _normalGeometry: Qt.rect(0, 0, 0, 0)
    function _trackNormalGeometry() {
        if (main.visibility === Window.Windowed)
            main._normalGeometry = Qt.rect(main.x, main.y, main.width, main.height)
    }
    onXChanged: main._trackNormalGeometry()
    onYChanged: main._trackNormalGeometry()
    onWidthChanged: main._trackNormalGeometry()
    onHeightChanged: main._trackNormalGeometry()
    onVisibilityChanged: main._trackNormalGeometry()
    onClosing: appContext.saveWindowGeometry(main._normalGeometry, main.visibility === Window.Maximized)

    KemaiContext {
        id: appContext
        onLoginSucceeded: session => {
            // Central 401 handling: an expired session drops back to the login view
            session.sessionExpired.connect(() => stackView.replace(null, loginViewComponent))
            stackView.replace(null, mainViewComponent, {
                sessionContext: session
            })
        }
        onLoggedOut: stackView.replace(null, loginViewComponent)
    }

    // Idle time and session lock, for the inactivity settings and stops
    PresenceMonitor {
        id: presenceMonitor
    }

    // GitHub releases: checked once at start, and on demand
    UpdateNotifier {
        id: updateNotifier
        preferences: appContext.preferences
    }

    SettingsDialog {
        id: settingsDialog
        kemaiContext: appContext
        presence: presenceMonitor
        updates: updateNotifier
        // A version found from Settings › Updates stays announced once the dialog is closed
        onClosed: updateNotifier.showInStrip()
    }

    AboutDialog {
        id: aboutDialog
    }

    LogJournalDialog {
        id: logJournalDialog
        journal: main.logJournal
    }

    Component {
        id: mainViewComponent
        MainView {
            preferences: appContext.preferences
            presence: presenceMonitor
            updates: updateNotifier
            settingsAvailable: settingsDialog.hasSections
            onSettingsRequested: settingsDialog.open()
            onLogJournalRequested: logJournalDialog.open()
            onAboutRequested: aboutDialog.open()
            // The session lives as long as its view: released once the view is gone (logout or expiry),
            // never while a transition still binds to it
            Component.onDestruction: appContext.closeSession(sessionContext)
            onLogoutRequested: appContext.logout()
        }
    }

    Component {
        id: loginViewComponent
        LoginView {
            kemaiContext: appContext
            updates: updateNotifier
            settingsAvailable: settingsDialog.hasSections
            onSettingsRequested: settingsDialog.open()
            onLogJournalRequested: logJournalDialog.open()
            onAboutRequested: aboutDialog.open()
        }
    }

    StackView {
        id: stackView
        anchors.fill: parent

        initialItem: loginViewComponent
    }

    Component.onCompleted: {
        appContext.showWindowWithSavedGeometry(main)
        appContext.initialize()
        updateNotifier.checkAtStart()
    }
}
