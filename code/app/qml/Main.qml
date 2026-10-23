// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Controls

import "Views" as Views

ApplicationWindow {

    id: main

    width: 800
    height: 600
    visible: true

    KemaiContext {
        id: kemaiContext
        onLoginSucceeded: (session) => {
            // Central 401 handling: an expired session drops back to the login view
            session.sessionExpired.connect(() => stackView.replace(null, loginViewComponent))
            stackView.replace(null, mainViewComponent, { sessionContext: session })
        }
    }

    Component {
        id: mainViewComponent
        Views.MainView {}
    }

    Component {
        id: loginViewComponent
        Views.LoginView { kemaiContext: kemaiContext }
    }

    StackView {
        id: stackView
        anchors.fill: parent

        initialItem: loginViewComponent
    }

    Component.onCompleted: kemaiContext.initialize()
}
