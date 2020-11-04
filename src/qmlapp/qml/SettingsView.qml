import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {

    ColumnLayout {

        anchors.fill: parent

        GridLayout {

            Layout.fillWidth: true
            Layout.fillHeight: true

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

        Item {

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignBottom

            RowLayout {

                anchors.fill: parent

                Button {
                    text: qsTr("Test")
                }
//                Item { Layout.fillWidth: true }
//                Button {
//                    text: qsTr("Cancel")
//                    Layout.alignment: Qt.AlignRight
//                }
//                Button {
//                    text: qsTr("Save")
//                    Layout.alignment: Qt.AlignRight
//                }
            }
        }
    }
}
