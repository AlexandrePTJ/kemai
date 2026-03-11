// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: root

    required property KemaiContext kemaiContext

    property bool loginPending: false
    property string loginError: ""

    Connections {
        target: root.kemaiContext
        function onLoginSucceeded() { root.loginPending = false }
        function onLoginFailed(error) { root.loginPending = false; root.loginError = error }
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
                Layout.fillWidth: true

                model: profileManager.model
                textRole: "name"
                valueRole: "profileId"

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
                    height: 1
                    Layout.fillWidth: true
                    color: Theme.colorSurface
                }

                Text {
                    text: "CONNECTION DETAILS"
                    color: Theme.colorTextSecondary
                    font.pixelSize: Theme.fontSizeXSmall
                }

                Rectangle {
                    height: 1
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
        }

        Item { Layout.fillHeight: true }

        // -- Save profile


        // -- Error message
        Text {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            visible: root.loginError.length > 0
            text: root.loginError
            color: "red"
            font.pixelSize: Theme.fontSizeSmall
            wrapMode: Text.WordWrap
        }

        // -- Connect Button
        Button {
            Layout.fillWidth: true
            font.pixelSize: Theme.fontSizeLarge
            font.bold: true
            implicitHeight: 48

            enabled: hostField.text && tokenField.text && !root.loginPending

            background: Rectangle {
                color: {
                    if (!enabled) {
                        Theme.colorAccentGreenDisabled
                    } else {
                        parent.down ? Theme.colorButtonStartDown : Theme.colorButtonStart
                    }
                }
                radius: Theme.radiusMedium
            }

            contentItem: Text {
                text: "Connect"
                color: Theme.colorTextPrimary
                font: parent.font
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: {
                root.loginError = ""
                root.loginPending = true
                root.kemaiContext.login(hostField.text, tokenField.text)
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
