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
                onClicked: settingsDrawer.visible = true
            }
            Item { Layout.fillWidth: true }
            ToolButton {
                icon.source: "qrc:/icons/refresh.svg"
            }
        }
    }

    ActivityView {
        anchors.fill: parent
        anchors.margins: 5
    }

    Drawer {
        id: settingsDrawer
        width: parent.width
        height: parent.height - header.height
        y: header.height

        SettingsView {
            anchors.fill: parent
            anchors.margins: 5

            onSaveActivated: settingsDrawer.visible = false
            onCancelActivated: settingsDrawer.visible = false
        }
    }
}
