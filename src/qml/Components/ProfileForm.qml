// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    property bool isEditing
    property var profileId: null
    property alias name: nameField.text
    property alias host: hostField.text
    property alias token: tokenField.text

    readonly property bool isValid: name.length > 0 && host.length > 0 && token.length > 0

    color: Theme.colorSurfaceLight
    radius: Theme.radiusLarge

    function reset() {
        profileId = null
        nameField.text = ""
        hostField.text = ""
        tokenField.text = ""
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: Theme.spacingMedium
        anchors.topMargin: Theme.spacingSmall
        anchors.rightMargin: Theme.spacingMedium

        spacing: Theme.spacingMedium

        Text {
            Layout.fillWidth: true

            text: root.isEditing ? "EDIT PROFILE" : "ADD PROFILE"
            color: Theme.colorTextSecondary
            font.pixelSize: Theme.fontSizeXSmall
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: Theme.spacingSmall

            Text {
                text: "PROFILE NAME"
                color: Theme.colorTextSecondary
                font.pixelSize: Theme.fontSizeXSmall
            }

            KemaiTextField {
                id: nameField
                Layout.fillWidth: true
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: Theme.spacingSmall

            Text {
                text: "HOST URL"
                color: Theme.colorTextSecondary
                font.pixelSize: Theme.fontSizeXSmall
            }

            KemaiTextField {
                id: hostField
                Layout.fillWidth: true
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: Theme.spacingSmall

            Text {
                text: "API TOKEN"
                color: Theme.colorTextSecondary
                font.pixelSize: Theme.fontSizeXSmall
            }

            KemaiTextField {
                id: tokenField
                Layout.fillWidth: true
                echoMode: TextInput.Password
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }
}