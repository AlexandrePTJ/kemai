import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Window

ApplicationWindow {
    id: window
    width: 500
    height: 400
    visible: true

    ListView {
        id: timeSheetHistoryView
        anchors.fill: parent
        model: TimeSheetModel {}
        delegate: TimeSheetDelegate {}
    }
}
