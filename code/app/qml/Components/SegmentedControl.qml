// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kemai

// One choice among a few, side by side; Left/Right move it when focused
FocusScope {
    id: root

    property list<string> options
    // Optional color square before each option's label (a transparent color: none)
    property list<color> swatches
    property int currentIndex: 0

    signal activated(int index)

    function _choose(index) {
        if (index >= 0 && index < root.options.length && index !== root.currentIndex)
            root.activated(index)
    }

    implicitHeight: 32
    implicitWidth: optionRow.implicitWidth + 4
    activeFocusOnTab: true

    Keys.onLeftPressed: root._choose(root.currentIndex - 1)
    Keys.onRightPressed: root._choose(root.currentIndex + 1)

    Rectangle {
        anchors.fill: parent
        radius: Theme.radiusMedium
        color: Theme.colorBackgroundDark
        border.width: 1
        border.color: root.activeFocus ? Theme.colorAccentGreenDim : Theme.colorBorderSubtle

        RowLayout {
            id: optionRow
            anchors.fill: parent
            anchors.margins: 2
            spacing: 0

            Repeater {
                model: root.options

                delegate: AbstractButton {
                    id: option

                    required property string modelData
                    required property int index

                    readonly property bool selected: option.index === root.currentIndex

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    leftPadding: 10
                    rightPadding: 10
                    focusPolicy: Qt.NoFocus
                    onClicked: root._choose(option.index)

                    HoverHandler {
                        cursorShape: Qt.PointingHandCursor
                    }

                    background: Rectangle {
                        radius: Theme.radiusSmall
                        color: option.selected ? Theme.colorSurface : "transparent"
                    }

                    contentItem: Item {
                        implicitWidth: optionContent.implicitWidth
                        implicitHeight: optionContent.implicitHeight

                        RowLayout {
                            id: optionContent
                            anchors.centerIn: parent
                            width: Math.min(optionContent.implicitWidth, parent.width)
                            spacing: 6

                            Rectangle {
                                visible: option.index < root.swatches.length && root.swatches[option.index].a > 0
                                Layout.preferredWidth: 7
                                Layout.preferredHeight: 7
                                radius: 2
                                color: option.index < root.swatches.length ? root.swatches[option.index] : "transparent"
                            }

                            Text {
                                Layout.fillWidth: true
                                text: option.modelData
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                color: option.selected ? Theme.colorTextPrimary : Theme.colorTextSecondary
                                font.pixelSize: Theme.fontSizeSmall
                                font.weight: Font.Medium
                                elide: Text.ElideRight
                            }
                        }
                    }
                }
            }
        }
    }
}
