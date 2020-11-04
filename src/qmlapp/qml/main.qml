import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

ApplicationWindow {
	width: 320
    height: 480
    visible: true

    ColumnLayout {

        anchors.fill: parent

        ActivityView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 10
        }

//        SettingsView {
//            Layout.fillWidth: true
//            Layout.fillHeight: true
//        }
    }
}
