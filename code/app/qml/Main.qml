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
        onLoginSucceeded: (session) => stackView.replace(null, mainViewComponent, { sessionContext: session })
    }

    Component {
        id: mainViewComponent
        Views.MainView {}
    }

    StackView {
        id: stackView
        anchors.fill: parent

        initialItem: Views.LoginView { kemaiContext: kemaiContext }
    }

    Component.onCompleted: kemaiContext.initialize()
}
