import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

ApplicationWindow {
	width: 240
    height: 320
    visible: true

    ColumnLayout {

        anchors.fill: parent

        ActivityView {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

//        SettingsView {
//            Layout.fillWidth: true
//            Layout.fillHeight: true
//        }
    }
}
