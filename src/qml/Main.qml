import QtQuick
import QtQuick.Controls

import "Views" as Views

ApplicationWindow {

    id: main

    width: 600
    height: 800
    visible: true

    StackView {
        id: stackView
        anchors.fill: parent

        initialItem: Views.LoginView { }
//        initialItem: Views.MainView { }
    }
}
