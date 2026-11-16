// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kemai

// History | Tasks switch of the timer hint line (only with the Task Management plugin)
Rectangle {
    id: root

    property string current: "history" // history | tasks
    property int taskCount: 0
    property bool taskCountVisible: false
    // Narrow window: icons only, the labels become tooltips
    property bool compact: false

    signal selected(string view)

    implicitWidth: switchRow.implicitWidth + 6
    implicitHeight: 28
    radius: Theme.radiusMedium
    color: Theme.colorBackgroundDark
    border.width: 1
    border.color: Theme.colorBorderSubtle

    RowLayout {
        id: switchRow
        anchors.fill: parent
        anchors.margins: 3
        spacing: 0

        Repeater {
            model: [
                {
                    key: "history",
                    label: "History",
                    icon: "clock"
                },
                {
                    key: "tasks",
                    label: "Tasks",
                    icon: "list"
                }
            ]

            delegate: AbstractButton {
                id: segment

                required property var modelData

                readonly property bool active: root.current === segment.modelData.key

                Layout.preferredHeight: 22
                leftPadding: root.compact ? 7 : 10
                rightPadding: root.compact ? 7 : 10
                focusPolicy: Qt.TabFocus
                onClicked: root.selected(segment.modelData.key)

                ToolTip.visible: root.compact && segment.hovered
                ToolTip.delay: 500
                ToolTip.text: segment.modelData.label

                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }

                background: Rectangle {
                    radius: Theme.radiusSmall
                    color: segment.active ? Theme.colorSurface : "transparent"
                    border.width: segment.visualFocus ? 1 : 0
                    border.color: Theme.colorAccentGreenDim
                }

                contentItem: RowLayout {
                    spacing: 5

                    Icon {
                        name: segment.modelData.icon
                        size: 12
                        color: segment.active ? Theme.colorAccentGreen : Theme.colorTextSecondary
                    }

                    Text {
                        visible: !root.compact
                        text: segment.modelData.label
                        color: segment.active ? Theme.colorTextPrimary : Theme.colorTextSecondary
                        font.pixelSize: Theme.fontSizeSmall
                        font.weight: Font.Medium
                    }

                    Rectangle {
                        visible: segment.modelData.key === "tasks" && root.taskCountVisible
                        Layout.preferredWidth: Math.max(Math.ceil(countText.implicitWidth) + 10, 16)
                        Layout.preferredHeight: 15
                        radius: 7.5
                        color: segment.active ? Theme.colorAccentGreenSoft : Theme.colorSurfaceDark

                        Text {
                            id: countText
                            anchors.centerIn: parent
                            text: root.taskCount
                            color: segment.active ? Theme.colorAccentGreen : Theme.colorTextSecondary
                            font.pixelSize: Theme.fontSizeTiny
                            font.weight: Font.DemiBold
                            font.features: Theme.fontFeaturesTabular
                        }
                    }
                }
            }
        }
    }
}
