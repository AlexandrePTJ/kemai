import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

ApplicationWindow {
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
                icon.source: "qrc:/icons/refresh.svg"
                onClicked: kimaiClientBridge.refresh()
            }
        }
    }

    RoundButton {
        anchors.fill: parent
        anchors.margins: 5

        Image {
            anchors.fill: parent
            anchors.margins: 5
            fillMode: Image.PreserveAspectFit

            source: kimaiClientBridge.timeSheetRunning ? "qrc:/icons/kimai-red.png" : "qrc:/icons/kimai-green.png"
        }

        onClicked: timesheetDrawer.open()
    }

    Drawer {
        id: settingsDrawer
        width: parent.width
        height: parent.height - header.height
        y: header.height
        edge: Qt.LeftEdge
        interactive: false

        SettingsView {
            anchors.fill: parent
            anchors.margins: 5
        }
    }

    Drawer {
        id: timesheetDrawer
        width: parent.width
        height: parent.height - header.height
        y: header.height
        edge: Qt.BottomEdge
        interactive: false

        TimesheetView {
            anchors.fill: parent
            anchors.margins: 5
        }
    }
}
