// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import "Views" as Views

ApplicationWindow {

    id: main

    width: 800
    height: 600
    visible: true
    title: appContext.windowTitle

    KemaiContext {
        id: appContext
        onLoginSucceeded: (session) => {
            // Central 401 handling: an expired session drops back to the login view
            session.sessionExpired.connect(() => stackView.replace(null, loginViewComponent))
            stackView.replace(null, mainViewComponent, { sessionContext: session })
        }
        onLoggedOut: stackView.replace(null, loginViewComponent)
    }

    Component {
        id: mainViewComponent
        Views.MainView {
            // The session lives as long as its view: released once the view is gone (logout or expiry),
            // never while a transition still binds to it
            Component.onDestruction: appContext.closeSession(sessionContext)
            onLogoutRequested: appContext.logout()
        }
    }

    Component {
        id: loginViewComponent
        Views.LoginView { kemaiContext: appContext }
    }

    StackView {
        id: stackView
        anchors.fill: parent

        initialItem: loginViewComponent
    }

    Component.onCompleted: appContext.initialize()
}
