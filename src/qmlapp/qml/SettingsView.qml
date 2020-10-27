import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15

Item {

    anchors.fill: parent

    ColumnLayout {
        anchors.fill: parent

        GridLayout {
            anchors.fill: parent
            anchors.bottom: undefined

            columns: 2
            flow: GridLayout.LeftToRight

            Label {
                text: qsTr("Host")
            }
            TextField {
                placeholderText: qsTr("Host")
                Layout.fillWidth: true
            }
            Label {
                text: qsTr("Username")
            }
            TextField {
                placeholderText: qsTr("Username")
                Layout.fillWidth: true
            }
            Label {
                text: qsTr("Token")
            }
            TextField {
                placeholderText: qsTr("Token")
                Layout.fillWidth: true
            }
            CheckBox {
                Layout.columnSpan: 2
                text: qsTr("Close to system tray")
            }
        }


        RowLayout {

            anchors.bottom: parent

            Button {
                text: qsTr("Test")
            }
            Item { Layout.fillWidth: true }
            Button {
                text: qsTr("Cancel")
            }
            Button {
                text: qsTr("Save")
            }
        }
    }
}
