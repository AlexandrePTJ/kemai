// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import Kemai

Rectangle {
    id: root

    property bool isEditing
    property var profileId: null
    property alias name: nameField.text
    property alias host: hostField.text
    property alias token: tokenField.text

    // Values of the edited profile as saved, to tell what changed
    property string savedName
    property string savedHost
    property string savedToken

    readonly property bool nameDiffers: root.isEditing && root.name.trim() !== root.savedName.trim()
    readonly property bool hostDiffers: root.isEditing && root.host.trim() !== root.savedHost.trim()
    readonly property bool tokenDiffers: root.isEditing && root.token !== root.savedToken
    readonly property bool hasChanges: root.nameDiffers || root.hostDiffers || root.tokenDiffers
    readonly property bool isComplete: root.name.trim().length > 0 && root.host.trim().length > 0 && root.token.trim().length > 0
    // Edit: something differs from the saved profile; add: every field filled in
    readonly property bool canSave: root.isComplete && (!root.isEditing || root.hasChanges)

    signal edited

    color: Theme.colorSurfaceLight
    radius: Theme.radiusLarge
    implicitHeight: formColumn.implicitHeight + Theme.spacingSmall + Theme.spacingMedium

    function load(editing, id, name, host, token) {
        root.isEditing = editing
        root.profileId = id
        root.savedName = name
        root.savedHost = host
        root.savedToken = token
        nameField.text = name
        hostField.text = host
        tokenField.text = token
    }

    function reset() {
        root.load(false, null, "", "", "")
    }

    component FieldLabel: RowLayout {
        id: fieldLabel

        property string text
        property bool changed

        spacing: 6

        Text {
            text: fieldLabel.text
            color: Theme.colorTextSecondary
            font.pixelSize: Theme.fontSizeXSmall
        }

        // Changed-field mark
        Rectangle {
            visible: fieldLabel.changed
            Layout.preferredWidth: 6
            Layout.preferredHeight: 6
            radius: 3
            color: Theme.colorAccentGreen
        }
    }

    ColumnLayout {
        id: formColumn

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: Theme.spacingMedium
        anchors.topMargin: Theme.spacingSmall
        anchors.rightMargin: Theme.spacingMedium

        spacing: 10

        Text {
            Layout.fillWidth: true

            text: root.isEditing ? "EDIT PROFILE" : "ADD PROFILE"
            color: Theme.colorTextSecondary
            font.pixelSize: Theme.fontSizeXSmall
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: Theme.spacingSmall

            FieldLabel {
                text: "PROFILE NAME"
                changed: root.nameDiffers
            }

            KemaiTextField {
                id: nameField
                Layout.fillWidth: true
                onTextEdited: root.edited()
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: Theme.spacingSmall

            FieldLabel {
                text: "HOST URL"
                changed: root.hostDiffers
            }

            KemaiTextField {
                id: hostField
                Layout.fillWidth: true
                onTextEdited: root.edited()
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: Theme.spacingSmall

            FieldLabel {
                text: "API TOKEN"
                changed: root.tokenDiffers
            }

            KemaiTextField {
                id: tokenField
                Layout.fillWidth: true
                echoMode: TextInput.Password
                onTextEdited: root.edited()
            }
        }
    }
}
