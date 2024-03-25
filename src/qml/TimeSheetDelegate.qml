import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Window

ItemDelegate {
    id: root
    width: parent.width

    contentItem: RowLayout {

        Label {
            id: activityNameLabel
            text: model.activityName
        }
        Label {
            id: projectNameLabel
            text: model.projectName
        }
        Item {
            Layout.fillWidth: true
        }
        Label {
            id: startDateTimeLabel
            text: model.startDateTime
        }
        Label {
            id: endDateTimeLabel
            text: model.endDateTime
        }
        Label {
            id: durationLabel
            text: model.duration
            font.bold: true
        }
        RoundButton {
            id: reloadButton
            width: 20
            height: 20
            icon.source: "qrc:/icons/refresh"
        }
        RoundButton {
            id: restartButton
            width: 20
            height: 20
            icon.source: "qrc:/icons/play"
        }
    }
}
