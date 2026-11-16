// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kemai

Item {
    id: root

    required property int index
    required property var profileId
    required property string name
    required property string host

    required property bool isSelected
    required property bool isLast
    // Just saved: inline "Saved" and an accent outline that fades out
    property bool justSaved: false

    signal selectionRequested(index: int)

    signal deleteConfirmRequested(profileId: var)

    width: ListView.view.width
    height: 62

    property bool isConfirming: false

    // Row background tint
    Rectangle {
        anchors.fill: parent
        color: {
            if (root.isConfirming)
                return Theme.colorAccentRedTint
            if (root.isSelected)
                return Theme.colorAccentGreenTint
            return "transparent"
        }
    }

    // Left accent bar (clipped by container radius on first/last rows)
    Rectangle {
        x: 0
        width: 3
        height: parent.height
        color: {
            if (root.isConfirming)
                return Theme.colorAccentRed
            if (root.isSelected)
                return Theme.colorAccentGreen
            return "transparent"
        }
    }

    // Just-saved outline
    Rectangle {
        anchors.fill: parent
        color: "transparent"
        radius: Theme.radiusMedium
        border.width: 1
        border.color: Theme.colorAccentGreenDim
        opacity: root.justSaved ? 1 : 0

        Behavior on opacity {
            NumberAnimation {
                duration: 600
            }
        }
    }

    // Click to select / deselect (behind the action buttons)
    TapHandler {
        gesturePolicy: TapHandler.ReleaseWithinBounds
        onTapped: root.selectionRequested(root.isSelected ? -1 : root.index)
    }

    HoverHandler {
        cursorShape: Qt.PointingHandCursor
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: Theme.spacingMedium
        anchors.rightMargin: Theme.spacingMedium
        spacing: Theme.spacingSmall

        // Name + host
        ColumnLayout {
            spacing: 2
            Layout.fillWidth: true

            RowLayout {
                spacing: 8

                Text {
                    text: root.name
                    color: Theme.colorTextPrimary
                    font.pixelSize: Theme.fontSizeNormal
                    font.bold: true
                }

                RowLayout {
                    visible: root.justSaved
                    spacing: 4

                    Icon {
                        name: "check"
                        size: 12
                        color: Theme.colorAccentGreen
                    }

                    Text {
                        text: "Saved"
                        color: Theme.colorAccentGreen
                        font.pixelSize: Theme.fontSizeXSmall
                        font.weight: Font.Medium
                    }
                }
            }

            Text {
                text: root.host
                color: Theme.colorTextSecondary
                font.pixelSize: Theme.fontSizeSmall
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
        }

        // Normal: trash icon
        AbstractButton {
            id: trashButton
            visible: !root.isConfirming
            implicitWidth: 28
            implicitHeight: 28

            contentItem: Item {
                Icon {
                    anchors.centerIn: parent
                    name: "trash-2"
                    size: 14
                    color: trashButton.hovered ? Theme.colorAccentRed : Theme.colorTextSecondary
                }
            }

            onClicked: root.isConfirming = true

            background: Rectangle {
                color: trashButton.hovered ? Theme.colorBackground : "transparent"
                radius: Theme.radiusLarge
            }
        }

        // Confirm: Remove? / Cancel / Remove
        RowLayout {
            visible: root.isConfirming
            spacing: Theme.spacingSmall

            Text {
                text: "Remove?"
                color: Theme.colorTextSecondary
                font.pixelSize: Theme.fontSizeSmall
            }

            PrimaryEditButton {
                implicitWidth: 60
                implicitHeight: 28
                text: "Cancel"

                onClicked: root.isConfirming = false
            }

            PrimaryRedButton {
                implicitWidth: 70
                implicitHeight: 28
                text: "Remove"

                onClicked: root.deleteConfirmRequested(root.profileId)
            }
        }
    }

    // Inter-row separator
    Rectangle {
        visible: !root.isLast
        anchors.bottom: parent.bottom
        width: parent.width
        height: 1
        color: Theme.colorBackground
    }
}
