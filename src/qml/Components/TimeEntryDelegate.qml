import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import kemai

Rectangle {
    id: root

    required property int index
    required property string status
    required property string activityColor
    required property string projectName
    required property string activityName
    required property string description
    required property string duration
    required property string timeRange

    width: ListView.view ? ListView.view.width : 0
    height: mainLayout.implicitHeight + 16
    color: status === "active" ? Theme.colorActiveEntry : "transparent"
    radius: 4

    // Left border strip
    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 3
        color: root.activityColor
        radius: 2
    }

    RowLayout {
        id: mainLayout
        anchors {
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
            leftMargin: 12
            rightMargin: 8
        }
        spacing: 10


        // Project / description column
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2

            RowLayout {
                spacing: 4
                Text {
                    text: root.projectName
                    color: Theme.colorTextPrimary
                    font.pixelSize: Theme.fontSizeNormal
                    font.bold: true
                }
                Text {
                    text: " - " + root.activityName
                    color: Theme.colorTextSecondary
                    font.pixelSize: Theme.fontSizeNormal
                }
            }

            Text {
                text: root.description
                color: Theme.colorTextSecondary
                font.pixelSize: Theme.fontSizeSmall
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
        }

        // Duration / time range column
        ColumnLayout {
            spacing: 2
            Layout.alignment: Qt.AlignVCenter

            Text {
                text: root.duration
                color: Theme.colorAccentGreen
                font.pixelSize: Theme.fontSizeNormal
                font.bold: true
                font.family: Theme.fontFamilyMono
                Layout.alignment: Qt.AlignRight
            }

            Text {
                text: root.timeRange
                color: Theme.colorTextSecondary
                font.pixelSize: Theme.fontSizeXSmall
                Layout.alignment: Qt.AlignRight
            }
        }

        // Play / Edit buttons (non-active entries only)
        RowLayout {
            spacing: 6
            visible: root.status !== "active"
            Layout.alignment: Qt.AlignVCenter

            Button {
                text: "✎ Edit"
                padding: 6
                font.pixelSize: Theme.fontSizeSmall
                background: Rectangle {
                    color: parent.down ? Theme.colorButtonEditDown : Theme.colorButtonEdit
                    radius: 4
                }
                contentItem: Text {
                    text: parent.text
                    color: Theme.colorTextPrimary
                    font: parent.font
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Button {
                text: "▶ Play"
                padding: 6
                font.pixelSize: Theme.fontSizeSmall
                background: Rectangle {
                    color: parent.down ? Theme.colorButtonStartDown : Theme.colorButtonStart
                    radius: 4
                }
                contentItem: Text {
                    text: parent.text
                    color: Theme.colorTextPrimary
                    font: parent.font
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }
}
