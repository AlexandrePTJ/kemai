// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Controls
import Kemai

Item {
    id: root

    required property ActivityListModel sourceModel
    property string placeholderText: ""
    property alias text: inputField.text

    signal activitySelected(string label)

    implicitHeight: 40

    ActivityFilterProxyModel {
        id: filterProxy
        sourceModel: root.sourceModel
        filterText: inputField.text
    }

    TextField {
        id: inputField
        anchors.fill: parent
        placeholderText: root.placeholderText
        placeholderTextColor: Theme.colorTextSecondary
        color: Theme.colorTextPrimary
        font.pixelSize: Theme.fontSizeNormal
        leftPadding: 10
        rightPadding: 10

        background: Rectangle {
            color: Theme.colorSurface
            radius: Theme.radiusMedium
            implicitHeight: 40
        }

        onActiveFocusChanged: {
            if (activeFocus) {
                dropdown.open()
            } else if (!dropdown.hovered) {
                dropdown.close()
            }
        }

        Keys.onEscapePressed: {
            dropdown.close()
            focus = false
        }
    }

    Popup {
        id: dropdown
        parent: root
        y: root.height + 2
        x: 0
        width: root.width
        padding: 4
        closePolicy: Popup.NoAutoClose

        background: Rectangle {
            color: Theme.colorSurface
            radius: Theme.radiusMedium
            border.color: Theme.colorBorderHighlight
            border.width: 1
        }

        contentItem: ListView {
            id: suggestionList
            clip: true
            model: filterProxy
            implicitHeight: Math.min(contentHeight, 240)

            delegate: ItemDelegate {
                id: entryDelegate

                required property int index
                required property string label
                required property string activityColor
                required property string projectName

                width: suggestionList.width
                highlighted: suggestionList.currentIndex === entryDelegate.index

                contentItem: Row {
                    spacing: 10
                    Rectangle {
                        width: 10
                        height: 10
                        radius: 5
                        color: entryDelegate.activityColor
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Text {
                        text: entryDelegate.label
                        color: Theme.colorTextPrimary
                        font.pixelSize: Theme.fontSizeNormal
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }
                }

                background: Rectangle {
                    color: entryDelegate.highlighted ? Theme.colorBackground : "transparent"
                    radius: Theme.radiusSmall
                }

                onClicked: {
                    inputField.text = entryDelegate.label
                    root.activitySelected(entryDelegate.label)
                    dropdown.close()
                    inputField.focus = false
                }
            }
        }
    }
}
