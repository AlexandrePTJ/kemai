import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

ApplicationWindow {

    readonly property int drawerHeight: height - header.height
    readonly property int drawerWidth: width
    readonly property int drawerY: header.height

	width: 320
    height: 480
    visible: true

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            ToolButton {
                icon.source: "qrc:/icons/settings.svg"
                onClicked: settingsDrawer.open()
            }

            Label {
                text: kimaiClientBridge.timeSheetDuration
                font.pointSize: 32
                font.bold: true
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter
                Layout.fillWidth: true
            }
            ToolButton {
                icon.source: "qrc:/icons/menu.svg"
                onClicked: menu.popup()
            }
        }

        Menu {
            id: menu
            MenuItem { text: qsTr("New Customer..."); onTriggered: customerDrawer.open(); }
            MenuItem { text: qsTr("New Project...") }
            MenuItem { text: qsTr("New Activity...") }
        }
    }

    Image {
        anchors.centerIn: parent
        anchors.margins: 5
        fillMode: Image.PreserveAspectFit

        source: kimaiClientBridge.timeSheetRunning ? "qrc:/icons/kimai-red.png" : "qrc:/icons/kimai-green.png"
        width: parent.width

        MouseArea {
            anchors.fill: parent
            onClicked: timesheetDrawer.open()
        }
    }

    Drawer {
        id: settingsDrawer
        width: drawerWidth
        height: drawerHeight
        y: drawerY
        edge: Qt.LeftEdge
        interactive: false

        SettingsView {
            anchors.fill: parent
            anchors.margins: 5
        }
    }

    Drawer {
        id: timesheetDrawer
        width: drawerWidth
        height: drawerHeight
        y: drawerY
        edge: Qt.BottomEdge
        interactive: false

        TimesheetView {
            anchors.fill: parent
            anchors.margins: 5
        }
    }

    Drawer {
        id: customerDrawer
        width: drawerWidth
        height: drawerHeight
        y: drawerY
        edge: Qt.RightEdge
        interactive: false

        Customer {
            anchors.fill: parent
            anchors.margins: 5
        }
    }
}
