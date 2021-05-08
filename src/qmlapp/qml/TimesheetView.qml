import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12


Item {

    ColumnLayout {

        anchors.fill: parent

        /*
         * Customers
         */
        RowLayout {
            Label { text: qsTr("Customer"); Layout.fillWidth: true }
            ToolButton { text: "+" }
        }
        ComboBox {
            id: customerComboBox
            enabled: !kimaiClientBridge.timeSheetRunning
            Layout.fillWidth: true
            model: customerDataModel
            valueRole: "kimaiId"
            textRole: "name"

            onCurrentValueChanged: projectDataModel.customerId = currentValue;
        }

        /*
         * Projects
         */
        RowLayout {
            Label { text: qsTr("Project"); Layout.fillWidth: true }
            ToolButton { text: "+" }
        }
        ComboBox {
            id: projectComboBox
            enabled: !kimaiClientBridge.timeSheetRunning && customerComboBox.currentIndex > 0
            Layout.fillWidth: true
            model: projectDataModel
            valueRole: "kimaiId"
            textRole: "name"

            onCurrentValueChanged: activityDataModel.projectId = currentValue;
        }

        /*
         * Activity
         */
        RowLayout {
            Label { text: qsTr("Activity"); Layout.fillWidth: true }
            ToolButton { text: "+" }
        }
        ComboBox {
            enabled: !kimaiClientBridge.timeSheetRunning && projectComboBox.currentIndex >= 0
            Layout.fillWidth: true
            model: activityDataModel
            valueRole: "kimaiId"
            textRole: "name"
        }

        //
        Item { Layout.fillHeight: true }

        Button {
            text: kimaiClientBridge.timeSheetRunning ? qsTr("Close") : qsTr("Go !")
            Layout.fillWidth: true
            onClicked: {
                if (!kimaiClientBridge.timeSheetRunning) {
                    // TODO: fill
                }
                timesheetDrawer.close()
            }
        }
    }
}
