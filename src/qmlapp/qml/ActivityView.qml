import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {

    id: activityItem

    ColumnLayout {

        anchors.fill: parent

        ScrollView {

            clip: true
            Layout.fillWidth: true
            Layout.fillHeight: true

            GridLayout {

                columns: 2
                flow: GridLayout.LeftToRight
                width: activityItem.width

                Label {
                    text: qsTr("Customer")
                }
                ComboBox {
                    Layout.fillWidth: true
                }
                Label {
                    text: qsTr("Project")
                }
                ComboBox {
                    Layout.fillWidth: true
                }
                Label {
                    text: qsTr("Activity")
                }
                ComboBox {
                    Layout.fillWidth: true
                }
                Label {
                    text: qsTr("Description")
                }
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    TextArea { }
                }
                Label {
                    text: qsTr("Tags")
                }
                TextArea {
                    Layout.fillWidth: true
                }
            }
        }

        Button {
            implicitHeight: 64
            Layout.fillWidth: true
        }
    }
}
