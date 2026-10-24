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

    signal settingsRequested()

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
            return "This server only answers over https: with http, Kemai could read but not save anything. "
                   + "<a href=\"https\"><font color=\"" + Theme.colorAccentGreen + "\"><b>Use " + detail + "</b></font></a>"
        case SessionController.UntrustedCertificate:
            return "The certificate of this server is not issued by an authority your system trusts."
        case SessionController.CertificateRejected:
            return "The certificate of this server is not valid (" + ({ expired: "expired or not yet valid", hostname: "issued for another name",
                                                                        revoked: "revoked" }[root.kemaiContext.refusedCertificate.rejection] || "refused") + ")."
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
        function onLoginSucceeded() { root.loginPending = false }
        function onLoginFailed(kind, detail) {
            root.loginPending = false
            root._httpsHost = kind === SessionController.HttpsRedirect ? detail : ""
            root.loginError = root._loginMessage(kind, detail)
        }
    }

    background: Rectangle {
        color: Theme.colorBackground
    }

    ProfileManager {
        id: profileManager
    }

    ProfileManagerDialog {
        id: profileManagerDialog
        profileManager: profileManager
    }

    ColumnLayout {
        anchors.fill: parent 
        anchors.margins: Theme.marginsMain
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

            Item { Layout.fillWidth: true }

            AbstractButton {
                id: settingsButton
                objectName: "loginSettingsButton"

                visible: root.settingsAvailable
                Layout.alignment: Qt.AlignTop
                Layout.preferredWidth: 28
                Layout.preferredHeight: 28
                focusPolicy: Qt.StrongFocus
                onClicked: root.settingsRequested()

                ToolTip.visible: settingsButton.hovered
                ToolTip.delay: 500
                ToolTip.text: "Settings"

                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }

                background: Rectangle {
                    radius: Theme.radiusMedium
                    color: settingsButton.hovered || settingsButton.visualFocus ? Theme.colorSurface : "transparent"
                    border.width: 1
                    border.color: Theme.colorBorderSubtle
                }

                contentItem: Item {
                    Icon {
                        anchors.centerIn: parent
                        name: "settings"
                        size: 15
                        color: settingsButton.hovered ? Theme.colorTextPrimary : Theme.colorTextSecondary
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
                    text: "<b><font color='" + Theme.colorTextPrimary + "'>Logged out of "
                          + root.kemaiContext.lastAccountName.replace(/&/g, "&amp;").replace(/</g, "&lt;")
                          + ".</font></b> Auto-login is off, so Kemai won't connect by itself next time."
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

                Item { Layout.fillWidth: true }

                Text {
                    text: "Manage profiles..."
                    color: Theme.colorAccentGreen
                    font.pixelSize: Theme.fontSizeSmall

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: profileManagerDialog.open()
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
                    const profile = profileManager.model.getById(currentValue);
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

                        contentItem: Text {
                            text: "👁"
                            color: showToken.checked ? Theme.colorTextTertiary : Theme.colorTextSecondary
                            font.pixelSize: 16
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
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

        Item { Layout.fillHeight: true }

        // -- Save profile


        // -- Error message
        Text {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            visible: root.loginError.length > 0
            text: root.loginError
            textFormat: Text.StyledText
            color: "red"
            font.pixelSize: Theme.fontSizeSmall
            wrapMode: Text.WordWrap
            // "Use https://…": fix the host, the user connects again
            onLinkActivated: {
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
                root.kemaiContext.login(hostField.text, tokenField.text, autoLoginCheckBox.checked,
                                        fromProfile ? profileCombo.currentText : "")
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
