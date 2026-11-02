// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kemai

import "../Components/ProfileSaveErrors.js" as ProfileSaveErrors

Popup {
    id: root

    required property ProfileManager profileManager

    // "idle" | "saving" | "saved" | "failed"
    property string _phase: "idle"
    // The save in flight or last answered: its id, the name and whether it adds a profile
    property var _pendingId: null
    property string _pendingName
    property bool _pendingIsNew
    property string _failedReason
    // Saving for more than about a second: a keychain prompt is probably open
    property bool _slow: false
    property bool _spinning: false
    // Selecting a row again after a reload keeps what the form holds
    property bool _keepForm: false

    readonly property bool _busy: root._phase === "saving"

    modal: true
    width: parent ? Math.min(560, parent.width - 40) : 560
    height: parent ? Math.min(root.implicitHeight, parent.height - 48) : root.implicitHeight
    x: parent ? (parent.width - width) / 2 : 0
    y: parent ? (parent.height - height) / 2 : 0
    padding: 0

    onClosed: {
        // A save still running finishes silently; its failure reopens the dialog with the form as typed
        if (root._busy)
            return
        root._phase = "idle"
        profileList.currentIndex = -1
        profileForm.reset()
    }

    function _sameId(a, b) {
        return a !== null && b !== null && String(a) === String(b)
    }

    function _loadForm(index) {
        if (index >= 0) {
            const profile = root.profileManager.model.getByIndex(index)
            profileForm.load(true, profile.id, profile.name, profile.host, profile.token)
        } else {
            profileForm.reset()
        }
    }

    function _save() {
        if (root._busy || !profileForm.canSave)
            return
        root._pendingName = profileForm.name.trim()
        root._pendingIsNew = !profileForm.isEditing
        root._phase = "saving"
        root._slow = false
        root._spinning = false
        spinnerDelay.restart()
        slowDelay.restart()
        root._pendingId = root.profileManager.upsert(profileForm.isEditing ? profileForm.profileId : "", root._pendingName, profileForm.host.trim(), profileForm.token)
    }

    // The list reloads after every save or removal: keep the edited profile selected by its id
    function _reselect() {
        const index = profileForm.isEditing ? root.profileManager.model.indexOf(profileForm.profileId) : -1
        if (index === profileList.currentIndex)
            return
        root._keepForm = index >= 0
        profileList.currentIndex = index
        root._keepForm = false
    }

    // Any edit ends the confirmation; a failure notice stays until the next save
    function _onEdited() {
        if (root._phase === "saved")
            root._phase = "idle"
    }

    Timer {
        id: spinnerDelay
        interval: 150
        onTriggered: root._spinning = true
    }

    Timer {
        id: slowDelay
        interval: 1000
        onTriggered: root._slow = true
    }

    Timer {
        id: savedDuration
        interval: 3000
        onTriggered: {
            if (root._phase === "saved")
                root._phase = "idle"
        }
    }

    Connections {
        target: root.profileManager.model

        function onModelReset() {
            root._reselect()
        }
    }

    Connections {
        target: root.profileManager

        function onSaveSucceeded(profileId) {
            if (!root._busy || !root._sameId(profileId, root._pendingId))
                return
            spinnerDelay.stop()
            slowDelay.stop()
            if (!root.opened) {
                root._phase = "idle"
                profileList.currentIndex = -1
                profileForm.reset()
                return
            }
            const index = root.profileManager.model.indexOf(profileId)
            if (index === profileList.currentIndex)
                root._loadForm(index)
            else
                profileList.currentIndex = index
            root._phase = "saved"
            savedDuration.restart()
        }

        function onSaveFailed(profileId, reason) {
            if (!root._busy || !root._sameId(profileId, root._pendingId))
                return
            spinnerDelay.stop()
            slowDelay.stop()
            root._failedReason = reason
            root._phase = "failed"
            if (!root.opened)
                root.open()
        }
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
            Layout.preferredHeight: 46
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

        // -- Profile list: the only part that scrolls, so the form and a failure notice stay in view
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: Theme.spacingLarge
            Layout.rightMargin: Theme.spacingLarge
            Layout.preferredHeight: profileList.contentHeight
            Layout.minimumHeight: Math.min(profileList.contentHeight, 62)

            visible: profileList.count > 0
            color: Theme.colorSurface
            radius: Theme.radiusMedium
            clip: true

            ListView {
                id: profileList
                anchors.fill: parent
                model: root.profileManager.model
                interactive: contentHeight > height
                boundsBehavior: Flickable.StopAtBounds
                currentIndex: -1

                WheelScroller {
                    flickable: profileList
                }

                delegate: ProfileEntryDelegate {
                    isSelected: profileList.currentIndex === index
                    isLast: profileList.count - 1 === index
                    justSaved: root._phase === "saved" && profileList.currentIndex === index

                    onSelectionRequested: index => {
                        if (!root._busy)
                            profileList.currentIndex = index
                    }
                    onDeleteConfirmRequested: profileId => root.profileManager.remove(profileId)
                }

                onCurrentIndexChanged: {
                    if (!root._keepForm) {
                        if (root._phase === "saved")
                            root._phase = "idle"
                        root._loadForm(currentIndex)
                    }
                    if (currentIndex >= 0)
                        positionViewAtIndex(currentIndex, ListView.Contain)
                }
            }
        }

        // -- Add / Edit form
        ProfileForm {
            id: profileForm

            Layout.fillWidth: true
            Layout.leftMargin: Theme.spacingLarge
            Layout.rightMargin: Theme.spacingLarge
            Layout.preferredHeight: implicitHeight

            enabled: !root._busy
            opacity: root._busy ? 0.7 : 1

            onEdited: root._onEdited()
        }

        // -- Failure notice, between the form and the footer
        Rectangle {
            Layout.fillWidth: true
            Layout.leftMargin: Theme.spacingLarge
            Layout.rightMargin: Theme.spacingLarge
            Layout.preferredHeight: noticeRow.implicitHeight + 14

            visible: root._phase === "failed"
            color: Theme.colorAccentRedSoft
            border.width: 1
            border.color: Theme.colorAccentRedLine
            radius: Theme.radiusMedium

            Accessible.role: Accessible.AlertMessage
            Accessible.name: noticeText.text

            RowLayout {
                id: noticeRow
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                anchors.topMargin: 7
                anchors.bottomMargin: 7
                spacing: 8

                Icon {
                    Layout.alignment: Qt.AlignTop
                    Layout.topMargin: 1
                    name: "circle-alert"
                    size: 16
                    color: Theme.colorAccentRed
                }

                Text {
                    id: noticeText
                    Layout.fillWidth: true
                    text: "<b>" + ProfileSaveErrors.reason(root._failedReason) + "</b> <font color=\"" + Theme.colorTextSecondary + "\">" + ProfileSaveErrors.consequence(root._failedReason, root._pendingName, root._pendingIsNew) + "</font>"
                    textFormat: Text.StyledText
                    wrapMode: Text.Wrap
                    color: Theme.colorTextPrimary
                    font.pixelSize: Theme.fontSizeSmall
                    lineHeight: 16
                    lineHeightMode: Text.FixedHeight
                }
            }
        }

        // -- Footer
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: Theme.spacingLarge
            Layout.rightMargin: Theme.spacingLarge
            Layout.bottomMargin: Theme.spacingLarge
            spacing: 10

            RowLayout {
                Layout.fillWidth: true
                spacing: 6

                Icon {
                    visible: root._busy && root._slow
                    name: "lock"
                    size: 13
                    color: Theme.colorTextSecondary
                }

                Icon {
                    visible: root._phase === "saved"
                    name: "check"
                    size: 13
                    color: Theme.colorAccentGreen
                }

                Text {
                    Layout.fillWidth: true
                    text: {
                        if (root._phase === "saved")
                            return "“" + root._pendingName + "” saved"
                        if (root._busy)
                            return root._slow ? "Waiting for the system keychain…" : ""
                        if (root._phase === "idle" && profileForm.hasChanges)
                            return "Unsaved changes"
                        return ""
                    }
                    color: root._phase === "saved" ? Theme.colorAccentGreen : Theme.colorTextSecondary
                    font.pixelSize: Theme.fontSizeSmall
                    elide: Text.ElideRight
                }
            }

            DialogButton {
                variant: "subtle"
                text: "Close"

                onClicked: root.close()
            }

            ProfileSaveButton {
                text: {
                    if (root._phase === "failed")
                        return profileForm.isEditing ? "Retry save" : "Retry add"
                    return profileForm.isEditing ? "Save" : "Add"
                }
                busy: root._busy
                spinning: root._spinning
                saved: root._phase === "saved"
                enabled: profileForm.canSave && !root._busy && root._phase !== "saved"

                onClicked: root._save()
            }
        }
    }
}
