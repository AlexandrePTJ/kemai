// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kemai

import "../Components"

// Asked only when a timer runs: logging out leaves it running on the server
Popup {
    id: root

    required property SessionContext sessionContext

    signal confirmed()

    parent: Overlay.overlay
    modal: true
    focus: true
    width: 420
    height: dialogColumn.implicitHeight
    x: parent ? (parent.width - width) / 2 : 0
    y: parent ? (parent.height - height) / 2 : 0
    padding: 0

    // Cancel is the safe default
    onOpened: cancelButton.forceActiveFocus(Qt.TabFocusReason)

    Overlay.modal: Rectangle {
        color: Theme.colorBackdrop
    }

    background: Rectangle {
        color: Theme.colorBackground
        border.color: Theme.colorBorderSubtle
        border.width: 1
        radius: Theme.radiusDialog
    }

    ColumnLayout {
        id: dialogColumn
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 0

        Text {
            Layout.fillWidth: true
            Layout.leftMargin: 18
            Layout.rightMargin: 18
            Layout.topMargin: 16
            text: "Log out while a timer is running?"
            color: Theme.colorTextPrimary
            font.pixelSize: Theme.fontSizeNormal
            font.weight: Font.DemiBold
            wrapMode: Text.Wrap
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.margins: 18
            Layout.topMargin: 14
            spacing: 12

            // Running entry
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 40
                radius: Theme.radiusMedium
                color: Theme.colorSurfaceDark
                border.width: 1
                border.color: Theme.colorBorderSubtle
                clip: true

                Rectangle {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    width: 3
                    color: root.sessionContext.activeActivityColor
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12
                    spacing: 6

                    Text {
                        text: root.sessionContext.activeProjectName
                        color: Theme.colorTextPrimary
                        font.pixelSize: Theme.fontSizeMedium
                        font.weight: Font.DemiBold
                    }

                    Text {
                        text: "—"
                        color: Theme.colorTextDim
                        font.pixelSize: Theme.fontSizeMedium
                    }

                    Text {
                        Layout.fillWidth: true
                        text: root.sessionContext.activeActivityName
                        color: Theme.colorTextSecondary
                        font.pixelSize: Theme.fontSizeMedium
                        elide: Text.ElideRight
                    }

                    Text {
                        text: root.sessionContext.activeTimesheetDuration
                        color: Theme.colorAccentGreen
                        font.pixelSize: Theme.fontSizeNormal
                        font.weight: Font.DemiBold
                        font.features: Theme.fontFeaturesTabular
                    }
                }
            }

            Text {
                Layout.fillWidth: true
                text: "It keeps running on the Kimai server and will show again when you log back in."
                color: Theme.colorTextPrimary
                font.pixelSize: Theme.fontSizeMedium
                lineHeight: 1.45
                wrapMode: Text.Wrap
            }

            Text {
                Layout.fillWidth: true
                text: "Auto-login will be turned off. Saved profiles are kept."
                color: Theme.colorTextSecondary
                font.pixelSize: Theme.fontSizeSmall
                wrapMode: Text.Wrap
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: Theme.colorDivider
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.margins: 18
            Layout.topMargin: 12
            Layout.bottomMargin: 12
            spacing: 8

            Item { Layout.fillWidth: true }

            DialogButton {
                id: cancelButton
                text: "Cancel"
                onClicked: root.close()
            }

            DialogButton {
                variant: "primary"
                iconName: "log-out"
                text: "Log out"
                onClicked: {
                    root.close()
                    root.confirmed()
                }
            }
        }
    }
}
