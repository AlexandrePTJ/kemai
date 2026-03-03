// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    required property int index
    required property var profileId
    required property string name
    required property string host

    required property bool isSelected
    required property bool isLast

    signal selectionRequested(index: int)

    signal deleteConfirmRequested(profileId: var)

    width: ListView.view.width
    height: 62

    property bool isConfirming: false

    // Row background tint
    Rectangle {
        anchors.fill: parent
        color: {
            if (root.isConfirming) return Theme.colorAccentRedTint
            if (root.isSelected) return Theme.colorAccentGreenTint
            return "transparent"
        }
    }

    // Left accent bar (clipped by container radius on first/last rows)
    Rectangle {
        x: 0
        width: 3
        height: parent.height
        color: {
            if (root.isConfirming) return Theme.colorAccentRed
            if (root.isSelected) return Theme.colorAccentGreen
            return "transparent"
        }
    }

    // Click to select / deselect (behind the action buttons)
    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: root.selectionRequested(root.isSelected ? -1 : root.index)
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

            Text {
                text: root.name
                color: Theme.colorTextPrimary
                font.pixelSize: Theme.fontSizeNormal
                font.bold: true
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
            visible: !root.isConfirming
            implicitWidth: 28
            implicitHeight: 28

            contentItem: Text {
                text: "🗑"
                font.pixelSize: Theme.fontSizeMedium
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: root.isConfirming = true

            background: Rectangle {
                color: parent.hovered ? Theme.colorBackground : "transparent"
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
        visible: !isLast
        anchors.bottom: parent.bottom
        width: parent.width
        height: 1
        color: Theme.colorBackground
    }
}