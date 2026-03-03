// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../Components"

Popup {
    id: root

    required property ProfileManager profileManager

    modal: true
    width: 480
    x: parent ? (parent.width - width) / 2 : 0
    y: parent ? (parent.height - height) / 2 : 0
    padding: 0

    onClosed: {
        profileList.currentIndex = -1
        profileForm.reset()
    }

    background: Rectangle {
        color: Theme.colorBackground
        radius: Theme.radiusLarge
    }

    ColumnLayout {

        anchors.fill: parent
        spacing: Theme.spacingMedium

        // -- Header
        Rectangle {
            Layout.fillWidth: true
            height: 52
            color: Theme.colorBackgroundDark
            radius: Theme.radiusLarge

            Text {
                anchors.left: parent.left
                anchors.leftMargin: Theme.spacingLarge
                anchors.verticalCenter: parent.verticalCenter

                text: "Manage Profiles"
                color: Theme.colorTextPrimary
                font.pixelSize: Theme.fontSizeNormal
                font.bold: true
            }
        }

        // -- Profile list
        Rectangle {
            Layout.fillWidth: true
            Layout.leftMargin: Theme.spacingLarge
            Layout.rightMargin: Theme.spacingLarge

            implicitHeight: profileList.contentHeight
            color: Theme.colorSurface
            radius: Theme.radiusMedium
            clip: true

            ListView {
                id: profileList
                anchors.fill: parent
                model: root.profileManager.model
                interactive: false

                delegate: ProfileEntryDelegate {
                    isSelected: profileList.currentIndex === index
                    isLast: profileList.model.count - 1 === index

                    onSelectionRequested: (index) => profileList.currentIndex = index
                    onDeleteConfirmRequested: (profileId) => root.profileManager.remove(profileId)
                }

                onCurrentIndexChanged: {
                    if (currentIndex >= 0) {
                        const profile = root.profileManager.model.getByIndex(currentIndex)

                        profileForm.isEditing = true
                        profileForm.profileId = profile.id
                        profileForm.name = profile.name
                        profileForm.host = profile.host
                        profileForm.token = profile.token
                    } else {
                        profileForm.isEditing = false
                        profileForm.profileId = null
                        profileForm.name = ""
                        profileForm.host = ""
                        profileForm.token = ""
                    }
                }
            }
        }

        // -- Add / Edit form
        ProfileForm {
            id: profileForm

            Layout.fillWidth: true
            Layout.leftMargin: Theme.spacingLarge
            Layout.rightMargin: Theme.spacingLarge

            height: 260
        }

        // -- Footer
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: Theme.spacingLarge
            Layout.rightMargin: Theme.spacingLarge
            Layout.topMargin: Theme.spacingMedium
            Layout.bottomMargin: Theme.spacingLarge
            spacing: Theme.spacingSmall

            Item {
                Layout.fillWidth: true
            }

            PrimaryEditButton {
                implicitWidth: 80
                implicitHeight: 40
                text: "Close"

                onClicked: root.close()
            }

            PrimaryRedButton {
                implicitWidth: 120
                implicitHeight: 40

                text: profileForm.isEditing ? "Save" : "Add"
                enabled: profileForm.isValid

                onClicked: profileManager.upsert(profileForm.profileId === null ? "" : profileForm.profileId, profileForm.name, profileForm.host, profileForm.token)
            }

            // AbstractButton {
            //     implicitWidth: 120
            //     implicitHeight: 40
            //     enabled: nameField.text.length > 0 && hostField.text.length > 0 && tokenField.text.length > 0
            //
            //     contentItem: Text {
            //         text: root.isEditing ? "Save Profile" : "Add Profile"
            //         color: Theme.colorTextPrimary
            //         font.pixelSize: Theme.fontSizeNormal
            //         font.bold: true
            //         horizontalAlignment: Text.AlignHCenter
            //         verticalAlignment: Text.AlignVCenter
            //         opacity: parent.enabled ? 1.0 : 0.5
            //     }
            //
            //     background: Rectangle {
            //         color: parent.hovered ? Theme.colorButtonStopDown : Theme.colorButtonStop
            //         radius: Theme.radiusMedium
            //         opacity: parent.enabled ? 1.0 : 0.5
            //     }
            //
            //     onClicked: {
            //         root.profileModel.upsert(root.selectedIndex, nameField.text, hostField.text, tokenField.text)
            //         root.selectedIndex = -1
            //     }
            // }
        }
    }
}
