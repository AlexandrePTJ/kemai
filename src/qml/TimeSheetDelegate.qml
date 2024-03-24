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
    }
}
