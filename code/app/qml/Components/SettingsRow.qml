// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Layouts

import Kemai

// One setting: title and description on the left, its control on the right. A disabled row
// greys out and gives the reason under it. Extra content (e.g. a stepper) goes under the description.
ColumnLayout {
    id: root

    property string title: ""
    property string description: ""
    // e.g. "Restart required"
    property string badge: ""
    // Set when the platform cannot honour the setting: the row is disabled and the reason shown
    property string unavailableReason: ""
    property bool last: false

    // The control at the right
    property alias control: controlSlot.data
    // Under the description
    default property alias extra: extraColumn.data

    readonly property bool available: root.unavailableReason === ""

    spacing: 0

    ColumnLayout {
        Layout.fillWidth: true
        Layout.topMargin: 14
        Layout.bottomMargin: 14
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            spacing: 16

            ColumnLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop
                spacing: 3
                opacity: root.available ? 1 : 0.55

                RowLayout {
                    spacing: 8

                    Text {
                        text: root.title
                        color: Theme.colorTextPrimary
                        font.pixelSize: Theme.fontSizeMedium
                        font.weight: Font.Medium
                    }

                    Rectangle {
                        visible: root.badge !== ""
                        Layout.preferredWidth: badgeText.implicitWidth + 12
                        Layout.preferredHeight: badgeText.implicitHeight + 2
                        radius: 3
                        color: Theme.colorWarningSoft
                        border.width: 1
                        border.color: Theme.colorWarningLine

                        Text {
                            id: badgeText
                            anchors.centerIn: parent
                            text: root.badge
                            color: Theme.colorWarning
                            font.pixelSize: 10
                            font.weight: Font.DemiBold
                            font.letterSpacing: 0.4
                        }
                    }
                }

                Text {
                    visible: root.description !== ""
                    Layout.fillWidth: true
                    text: root.description
                    color: Theme.colorTextSecondary
                    font.pixelSize: Theme.fontSizeSmall
                    // 12 px at 1.45
                    lineHeightMode: Text.FixedHeight
                    lineHeight: 17
                    wrapMode: Text.Wrap
                }

                ColumnLayout {
                    id: extraColumn
                    Layout.fillWidth: true
                    spacing: 0
                }
            }

            Item {
                id: controlSlot
                Layout.alignment: Qt.AlignTop
                Layout.preferredWidth: controlSlot.childrenRect.width
                Layout.preferredHeight: controlSlot.childrenRect.height
                enabled: root.available
            }
        }

        RowLayout {
            visible: !root.available
            Layout.fillWidth: true
            Layout.topMargin: 6
            spacing: 6

            Icon {
                Layout.alignment: Qt.AlignTop
                Layout.topMargin: 1
                name: "info"
                size: 13
                color: Theme.colorTextSecondary
            }

            Text {
                Layout.fillWidth: true
                text: root.unavailableReason
                color: Theme.colorTextSecondary
                font.pixelSize: Theme.fontSizeSmall
                wrapMode: Text.Wrap
            }
        }
    }

    Rectangle {
        visible: !root.last
        Layout.fillWidth: true
        Layout.preferredHeight: 1
        color: Theme.colorDivider
    }
}
