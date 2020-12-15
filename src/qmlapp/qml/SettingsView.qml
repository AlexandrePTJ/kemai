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
                text: settingsViewBridge.host
                onTextChanged: settingsViewBridge.host = text;
                Layout.fillWidth: true
            }
            Label {
                text: qsTr("Username")
            }
            TextField {
                placeholderText: qsTr("Username")
                text: settingsViewBridge.username
                onTextChanged: settingsViewBridge.username = text;
                Layout.fillWidth: true
            }
            Label {
                text: qsTr("Token")
            }
            TextField {
                placeholderText: qsTr("Token")
                text: settingsViewBridge.token
                onTextChanged: settingsViewBridge.token = text;
                Layout.fillWidth: true
            }
            CheckBox {
                Layout.columnSpan: 2
                text: qsTr("Close to system tray")
            }
        }

        Item { Layout.fillHeight: true }

        Label {
            id: testResult
            wrapMode: Text.Wrap

            Layout.fillWidth: true
        }

        RowLayout {

            Layout.fillWidth: true

            Button {
                text: qsTr("Test")
                onClicked: {
                    testResult.text = "";
                    settingsViewBridge.test();
                }
            }
            Item { Layout.fillWidth: true }
            Button {
                text: qsTr("Cancel")
                Layout.alignment: Qt.AlignRight
                onClicked: settingsDrawer.close()
            }
            Button {
                text: qsTr("Save")
                Layout.alignment: Qt.AlignRight
                onClicked: {
                    // TODO: impl save
                    settingsDrawer.close()
                }
            }
        }
    }

    Connections {
        target: settingsViewBridge
        function onTestFinished(ok, message) {
            testResult.text = message;
            testResult.color = ok ? "green" : "red";
        }
    }
}
