// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kemai

Page {
    id: root

    required property KemaiContext kemaiContext

    // At least one settings section exists: the settings button shows
    property bool settingsAvailable: false
    required property UpdateNotifier updates

    signal settingsRequested
    signal logJournalRequested
    signal aboutRequested

    property bool loginPending: false
    property string loginError: ""
    // https address to use instead of the http:// host typed (HttpsRedirect)
    property string _httpsHost: ""

    function _loginMessage(kind, detail) {
        switch (kind) {
        case SessionController.AuthFailed:
            return "Kimai refused the API token. Check it in your Kimai profile, under API access."
        case SessionController.NoNetwork:
            return "Can't reach this address. Check the host and your connection."
        case SessionController.NotFound:
            return "No Kimai API at this address. Check the host, e.g. https://kimai.example.com."
        case SessionController.HttpsRedirect:
            return "This server only answers over https: with http, Kemai could read but not save anything. " + "<a href=\"https\"><font color=\"" + Theme.colorAccentGreen + "\"><b>Use " + detail + "</b></font></a>"
        case SessionController.Forbidden:
            return "This Kimai account isn't allowed to use the API."
        case SessionController.ServerError:
            return "The Kimai server returned an error. Try again in a moment."
        case SessionController.InvalidResponse:
            return "The server answered, but not like Kimai. Check the host."
        default:
            return detail !== "" ? "Couldn't connect: " + detail : "Couldn't connect."
        }
    }

    Connections {
        target: root.kemaiContext
        function onLoginSucceeded() {
            root.loginPending = false
        }
        function onLoginFailed(kind, detail) {
            root.loginPending = false
            // A certificate problem is explained by the prompt, not by an error line
            if (kind === SessionController.UntrustedCertificate || kind === SessionController.CertificateRejected) {
                root.loginError = ""
                certificatePrompt.openFor(root.kemaiContext.refusedCertificate)
                return
            }
            root._httpsHost = kind === SessionController.HttpsRedirect ? detail : ""
            root.loginError = root._loginMessage(kind, detail) + " <a href=\"log\"><font color=\"" + Theme.colorAccentGreen + "\">Show log</font></a>"
        }
    }

    background: Rectangle {
        color: Theme.colorBackground
    }

    ProfileManager {
        id: profileManager
    }

    CertificatePrompt {
        id: certificatePrompt
        onTrustRequested: {
            root.loginPending = true
            root.kemaiContext.trustRefusedCertificate()
        }
    }

    ProfileManagerDialog {
        id: profileManagerDialog
        profileManager: profileManager
    }

    // A newer Kemai, or the result of a check started from the menu
    UpdateStrip {
        id: updateStrip
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        notifier: root.updates
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.marginsMain
        anchors.topMargin: Theme.marginsMain + (updateStrip.visible ? updateStrip.height : 0)
        spacing: Theme.spacingLarge

        // -- Logo & Title
        RowLayout {
            spacing: Theme.spacingMedium

            Image {
                source: "qrc:/assets/images/kimai_logo.svg"
                sourceSize: Qt.size(56, 56)
                fillMode: Image.PreserveAspectFit
            }

            ColumnLayout {
                spacing: 2

                Text {
                    text: "Kemai"
                    color: Theme.colorTextPrimary
                    font.pixelSize: Theme.fontSizeXLarge
                    font.bold: true
                }

                Text {
                    text: "Kimai desktop client"
                    color: Theme.colorTextSecondary
                    font.pixelSize: Theme.fontSizeSmall
                }
            }

            Item {
                Layout.fillWidth: true
            }

            // Help menu: what can be done before any session
            AbstractButton {
                id: moreButton
                objectName: "loginMoreButton"

                Layout.alignment: Qt.AlignTop
                Layout.preferredWidth: 28
                Layout.preferredHeight: 28
                focusPolicy: Qt.StrongFocus
                onClicked: loginMenu.opened ? loginMenu.close() : loginMenu.open()

                ToolTip.visible: moreButton.hovered && !loginMenu.opened
                ToolTip.delay: 500
                ToolTip.text: "More"

                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }

                background: Rectangle {
                    radius: Theme.radiusMedium
                    color: loginMenu.opened || moreButton.hovered || moreButton.visualFocus ? Theme.colorSurface : "transparent"
                    border.width: 1
                    border.color: loginMenu.opened ? Theme.colorAccentGreenDim : Theme.colorBorderSubtle
                }

                contentItem: Item {
                    Icon {
                        anchors.centerIn: parent
                        name: "ellipsis"
                        size: 15
                        color: moreButton.hovered || loginMenu.opened ? Theme.colorTextPrimary : Theme.colorTextSecondary
                    }
                }

                Popup {
                    id: loginMenu

                    y: moreButton.height + 6
                    x: moreButton.width - loginMenu.width
                    width: 230
                    padding: 4
                    focus: true
                    // Pressing the button itself toggles the menu
                    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

                    onOpened: (settingsEntry.visible ? settingsEntry : logJournalEntry).forceActiveFocus()

                    background: Rectangle {
                        color: Theme.colorBackground
                        radius: Theme.radiusMedium
                        border.width: 1
                        border.color: Theme.colorBorderSubtle
                    }

                    contentItem: ColumnLayout {
                        spacing: 0

                        MenuEntry {
                            id: settingsEntry
                            objectName: "loginSettingsEntry"
                            visible: root.settingsAvailable
                            iconName: "settings"
                            text: "Settings…"
                            onClicked: {
                                loginMenu.close()
                                root.settingsRequested()
                            }
                        }

                        MenuEntry {
                            id: logJournalEntry
                            objectName: "loginLogJournalEntry"
                            iconName: "list"
                            text: "Log journal…"
                            onClicked: {
                                loginMenu.close()
                                root.logJournalRequested()
                            }
                        }

                        MenuEntry {
                            objectName: "loginCheckUpdatesEntry"
                            visible: root.updates.enabled && !root.updates.devBuild
                            iconName: "refresh-cw"
                            text: "Check for updates"
                            onClicked: {
                                loginMenu.close()
                                root.updates.checkNow(UpdateNotifier.Menu)
                            }
                        }

                        MenuEntry {
                            objectName: "loginAboutEntry"
                            iconName: "info"
                            text: "About Kemai"
                            onClicked: {
                                loginMenu.close()
                                root.aboutRequested()
                            }
                        }
                    }
                }
            }
        }

        // -- After a logout: why Kemai did not connect by itself
        Rectangle {
            visible: root.kemaiContext.justLoggedOut
            Layout.fillWidth: true
            Layout.preferredHeight: logoutNotice.implicitHeight + 20
            radius: Theme.radiusMedium
            color: Theme.colorSurfaceDark
            border.width: 1
            border.color: Theme.colorBorderSubtle

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 10

                Icon {
                    Layout.alignment: Qt.AlignTop
                    name: "info"
                    size: 15
                    color: Theme.colorAccentGreen
                }

                Text {
                    id: logoutNotice
                    Layout.fillWidth: true
                    text: "<b><font color='" + Theme.colorTextPrimary + "'>Logged out of " + root.kemaiContext.lastAccountName.replace(/&/g, "&amp;").replace(/</g, "&lt;") + ".</font></b> Auto-login is off, so Kemai won't connect by itself next time."
                    textFormat: Text.StyledText
                    color: Theme.colorTextSecondary
                    font.pixelSize: Theme.fontSizeSmall
                    wrapMode: Text.Wrap
                }
            }
        }

        // -- Profile
        ColumnLayout {
            Layout.fillWidth: true
            spacing: Theme.spacingSmall

            RowLayout {
                Layout.fillWidth: true

                Text {
                    text: "PROFILE"
                    color: Theme.colorTextSecondary
                    font.pixelSize: Theme.fontSizeXSmall
                }

                Item {
                    Layout.fillWidth: true
                }

                // A button, not a TapHandler on the text: the page accepts presses and took them over
                AbstractButton {
                    focusPolicy: Qt.NoFocus
                    onClicked: profileManagerDialog.open()

                    contentItem: Text {
                        text: "Manage profiles..."
                        color: Theme.colorAccentGreen
                        font.pixelSize: Theme.fontSizeSmall
                    }

                    HoverHandler {
                        cursorShape: Qt.PointingHandCursor
                    }
                }
            }

            ProfileComboBox {
                id: profileCombo
                Layout.fillWidth: true

                model: profileManager.model
                textRole: "name"
                valueRole: "profileId"

                // Back from a session: keep the profile it was opened from
                function selectLastProfile() {
                    const index = profileCombo.find(root.kemaiContext.lastAccountName)
                    if (index >= 0)
                        profileCombo.currentIndex = index
                }
                Component.onCompleted: profileCombo.selectLastProfile()
                onCountChanged: profileCombo.selectLastProfile()

                onCurrentValueChanged: {
                    const profile = profileManager.model.getById(currentValue)
                    hostField.text = profile.host
                    tokenField.text = profile.token
                }
            }
        }

        // -- Connection Details
        ColumnLayout {
            Layout.fillWidth: true
            spacing: Theme.spacingMedium

            RowLayout {
                Layout.fillWidth: true
                spacing: Theme.spacingSmall

                Rectangle {
                    Layout.preferredHeight: 1
                    Layout.fillWidth: true
                    color: Theme.colorSurface
                }

                Text {
                    text: "CONNECTION DETAILS"
                    color: Theme.colorTextSecondary
                    font.pixelSize: Theme.fontSizeXSmall
                }

                Rectangle {
                    Layout.preferredHeight: 1
                    Layout.fillWidth: true
                    color: Theme.colorSurface
                }
            }

            // Host URL
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

            // API Token
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
                    echoMode: showToken.checked ? TextInput.Normal : TextInput.Password

                    AbstractButton {
                        id: showToken
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.rightMargin: 10
                        width: 24
                        height: 24
                        checkable: true

                        contentItem: Item {
                            Icon {
                                anchors.centerIn: parent
                                name: showToken.checked ? "eye-off" : "eye"
                                size: 16
                                color: showToken.checked ? Theme.colorTextTertiary : Theme.colorTextSecondary
                            }
                        }

                        background: Item {}
                    }
                }
            }

            CheckBox {
                id: autoLoginCheckBox
                text: "Auto-login"
            }
        }

        Item {
            Layout.fillHeight: true
        }

        // -- Save profile

        // -- Error message
        Text {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            visible: root.loginError.length > 0
            text: root.loginError
            textFormat: Text.StyledText
            color: Theme.colorAccentRed
            font.pixelSize: Theme.fontSizeSmall
            wrapMode: Text.WordWrap
            // "Show log": the failure's technical detail; "Use https://…": fix the host, the user connects again
            onLinkActivated: link => {
                if (link === "log") {
                    root.logJournalRequested()
                    return
                }
                hostField.text = root._httpsHost
                root.loginError = ""
            }

            HoverHandler {
                cursorShape: parent.hoveredLink !== "" ? Qt.PointingHandCursor : Qt.ArrowCursor
            }
        }

        // -- Connect Button
        Button {
            id: connectButton
            Layout.fillWidth: true
            font.pixelSize: Theme.fontSizeLarge
            font.bold: true
            implicitHeight: 48

            enabled: hostField.text && tokenField.text && !root.loginPending

            background: Rectangle {
                color: {
                    if (!connectButton.enabled) {
                        Theme.colorAccentGreenDisabled
                    } else {
                        connectButton.down ? Theme.colorButtonStartDown : Theme.colorButtonStart
                    }
                }
                radius: Theme.radiusMedium
            }

            contentItem: Text {
                text: "Connect"
                color: Theme.colorTextPrimary
                font: connectButton.font
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: {
                root.loginError = ""
                root.loginPending = true
                // Name the session after the selected profile only if its credentials are the ones used
                const profile = profileCombo.currentIndex >= 0 ? profileManager.model.getById(profileCombo.currentValue) : null
                const fromProfile = profile && profile.host === hostField.text && profile.token === tokenField.text
                root.kemaiContext.login(hostField.text, tokenField.text, autoLoginCheckBox.checked, fromProfile ? profileCombo.currentText : "")
            }
        }

        // -- Footer
        Text {
            Layout.alignment: Qt.AlignHCenter
            text: "Credentials are stored locally on this device."
            color: Theme.colorTextSecondary
            font.pixelSize: Theme.fontSizeXSmall
        }
    }
}
