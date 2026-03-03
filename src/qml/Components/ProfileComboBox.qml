import QtQuick
import QtQuick.Controls

ComboBox {
    id: root

    background: Rectangle {
        color: Theme.colorSurface
        radius: Theme.radiusMedium
        implicitHeight: 40
    }

    contentItem: Text {
        leftPadding: 10
        rightPadding: 30
        text: root.displayText
        color: Theme.colorTextPrimary
        font.pixelSize: Theme.fontSizeNormal
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    delegate: ItemDelegate {
        width: root.width
        highlighted: root.highlightedIndex === index
        contentItem: Text {
            text: model.name
            color: Theme.colorTextPrimary
            font.pixelSize: Theme.fontSizeNormal
            verticalAlignment: Text.AlignVCenter
        }
        background: Rectangle {
            color: highlighted ? Theme.colorBackground : "transparent"
        }
    }

    popup: Popup {
        y: root.height + 2
        width: root.width
        padding: 4
        background: Rectangle {
            color: Theme.colorSurfaceLight
            radius: Theme.radiusMedium
        }
        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: root.popup.visible ? root.delegateModel : null
            currentIndex: root.highlightedIndex
        }
    }
}
