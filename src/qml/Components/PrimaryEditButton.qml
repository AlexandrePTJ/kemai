import QtQuick
import QtQuick.Controls

AbstractButton {
    id: root

    contentItem: Text {
        id: contentText
        text: root.text
        color: Theme.colorTextPrimary
        font.pixelSize: Theme.fontSizeSmall
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        color: parent.hovered ? Theme.colorButtonEditDown : Theme.colorButtonEdit
        radius: Theme.radiusSmall
    }
}
