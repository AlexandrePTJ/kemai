// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kemai

// Shown when a login meets a server certificate the system does not trust: trust it for this server
// (first time, or a changed certificate), or learn why it can never be trusted. Cancel is the default:
// it has the focus, and Enter / Esc cancel. Nothing is stored unless the user trusts.
Popup {
    id: root

    // KemaiContext::refusedCertificate
    property var certificate: ({})

    signal trustRequested

    readonly property string _kind: root.certificate.rejection !== undefined && root.certificate.rejection !== "none" ? "refused" : root.certificate.previousSha256 !== undefined && root.certificate.previousSha256 !== "" ? "changed" : "first"
    readonly property string _host: root.certificate.host !== undefined ? root.certificate.host : ""
    property bool _detailsOpen: false

    function openFor(certificate) {
        root.certificate = certificate
        root._detailsOpen = root._kind === "refused"
        root.open()
    }

    function _date(value) {
        return value !== undefined ? Qt.formatDate(value, Locale.LongFormat) : ""
    }

    // Plain-language reason of a refused certificate, key part in bold
    readonly property string _reason: {
        const bold = text => "<font color=\"" + Theme.colorTextPrimary + "\"><b>" + text + "</b></font>"
        switch (root.certificate.rejection) {
        case "expired":
            return "Its certificate " + bold("expired on " + root._date(root.certificate.validUntil)) + ". The server's administrator needs to renew it."
        case "notyetvalid":
            return "Its certificate " + bold("isn't valid until " + root._date(root.certificate.validFrom)) + ". Check this computer's date and time, or ask the administrator."
        case "hostname":
            return "Its certificate was " + bold("issued for another server") + " (" + root.certificate.subject + "). Check the host URL, or ask the administrator."
        case "revoked":
            return "Its certificate " + bold("has been revoked") + " by its issuer. Ask the server's administrator."
        default:
            return "Its certificate " + bold("can't be verified") + ". Ask the server's administrator."
        }
    }

    parent: Overlay.overlay
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape
    width: Math.min(560, parent ? parent.width - 40 : 560)
    height: Math.min(promptColumn.implicitHeight, parent ? parent.height - 40 : 560)
    x: parent ? (parent.width - width) / 2 : 0
    y: parent ? (parent.height - height) / 2 : 0
    padding: 0

    // Cancel (or Close) is the default action
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

    contentItem: ColumnLayout {
        id: promptColumn
        spacing: 0

        // -- Head
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            Layout.topMargin: 18
            spacing: 14

            Rectangle {
                Layout.alignment: Qt.AlignTop
                Layout.preferredWidth: 40
                Layout.preferredHeight: 40
                radius: Theme.radiusDialog
                color: root._kind === "refused" ? Theme.colorAccentRedSoft : Theme.colorWarningSoft
                border.width: 1
                border.color: root._kind === "refused" ? Theme.colorAccentRedLine : Theme.colorWarningLine

                Icon {
                    anchors.centerIn: parent
                    name: root._kind === "refused" ? "x" : "shield-alert"
                    size: 20
                    color: root._kind === "refused" ? Theme.colorAccentRed : Theme.colorWarning
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 6

                Text {
                    Layout.fillWidth: true
                    text: root._kind === "refused" ? "Can't connect securely to " + root._host : root._kind === "changed" ? "The certificate of " + root._host + " has changed" : "Do you trust " + root._host + "?"
                    color: Theme.colorTextPrimary
                    font.pixelSize: Theme.fontSizeLarge
                    font.weight: Font.DemiBold
                    wrapMode: Text.Wrap
                }

                Text {
                    Layout.fillWidth: true
                    textFormat: Text.StyledText
                    text: root._kind === "refused" ? root._reason : root._kind === "changed" ? "You trusted a different certificate for this server before. This happens when the certificate is renewed — but it can also mean someone is intercepting the connection." : "Its certificate isn't issued by an authority your system trusts. That's common for servers run at home or in a small company."
                    color: Theme.colorTextSecondary
                    font.pixelSize: Theme.fontSizeMedium
                    lineHeightMode: Text.FixedHeight
                    lineHeight: 19
                    wrapMode: Text.Wrap
                }
            }
        }

        // -- Body (scrolls when the window is short)
        Flickable {
            id: bodyFlick
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: bodyColumn.implicitHeight + 32
            clip: true
            contentWidth: width
            contentHeight: bodyColumn.implicitHeight + 32
            boundsBehavior: Flickable.StopAtBounds
            interactive: contentHeight > height

            WheelScroller {
                flickable: bodyFlick
            }

            ColumnLayout {
                id: bodyColumn
                x: 20
                y: 16
                width: bodyFlick.width - 40
                spacing: 10

                FingerprintBlock {
                    visible: root._kind === "changed"
                    Layout.fillWidth: true
                    label: "Previously trusted"
                    sha256: root.certificate.previousSha256 !== undefined ? root.certificate.previousSha256 : ""
                    superseded: true
                    copyable: false
                }

                FingerprintBlock {
                    visible: root._kind !== "refused"
                    Layout.fillWidth: true
                    label: root._kind === "changed" ? "New certificate" : "SHA-256 fingerprint"
                    sha256: root.certificate.sha256 !== undefined ? root.certificate.sha256 : ""
                    highlighted: root._kind === "changed"
                }

                // Advice
                RowLayout {
                    visible: root._kind !== "refused"
                    Layout.fillWidth: true
                    spacing: 8

                    Icon {
                        Layout.alignment: Qt.AlignTop
                        Layout.topMargin: 2
                        name: "info"
                        size: 14
                        color: Theme.colorTextSecondary
                    }

                    Text {
                        Layout.fillWidth: true
                        textFormat: Text.StyledText
                        text: (root._kind === "changed" ? "Only continue if the administrator confirms the new fingerprint, or the browser shows the same one. " : "Compare it with the fingerprint given by the server's administrator, or shown by your browser for this site. ") + "If you trust it, Kemai accepts <font color=\"" + Theme.colorTextPrimary + "\">this exact certificate, for this server only</font>, and asks again if it changes."
                        color: Theme.colorTextSecondary
                        font.pixelSize: Theme.fontSizeSmall
                        lineHeightMode: Text.FixedHeight
                        lineHeight: 18
                        wrapMode: Text.Wrap
                    }
                }

                // Certificate details
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: detailsColumn.implicitHeight
                    radius: Theme.radiusMedium
                    color: "transparent"
                    border.width: 1
                    border.color: Theme.colorBorderSubtle

                    ColumnLayout {
                        id: detailsColumn
                        anchors.left: parent.left
                        anchors.right: parent.right
                        spacing: 0

                        AbstractButton {
                            id: detailsToggle
                            Layout.fillWidth: true
                            padding: 8
                            leftPadding: 12
                            focusPolicy: Qt.StrongFocus
                            onClicked: root._detailsOpen = !root._detailsOpen

                            HoverHandler {
                                cursorShape: Qt.PointingHandCursor
                            }

                            background: Rectangle {
                                color: "transparent"
                                radius: Theme.radiusMedium
                                border.width: detailsToggle.visualFocus ? 1 : 0
                                border.color: Theme.colorAccentGreenDim
                            }

                            contentItem: RowLayout {
                                spacing: 7

                                Icon {
                                    name: root._detailsOpen ? "chevron-down" : "chevron-right"
                                    size: 13
                                    color: Theme.colorTextSecondary
                                }

                                Text {
                                    text: "Certificate details"
                                    color: Theme.colorTextSecondary
                                    font.pixelSize: Theme.fontSizeSmall
                                }

                                Item {
                                    Layout.fillWidth: true
                                }
                            }
                        }

                        GridLayout {
                            visible: root._detailsOpen
                            Layout.fillWidth: true
                            Layout.leftMargin: 33
                            Layout.rightMargin: 12
                            Layout.topMargin: 2
                            Layout.bottomMargin: 12
                            columns: 2
                            columnSpacing: 10
                            rowSpacing: 6

                            DetailLabel {
                                text: "Server"
                            }
                            DetailValue {
                                text: root._host
                            }

                            DetailLabel {
                                text: "Issued to"
                            }
                            DetailValue {
                                faulty: root.certificate.rejection === "hostname"
                                text: (root.certificate.subject !== undefined ? root.certificate.subject : "") + (faulty ? " · not this server" : "")
                            }

                            DetailLabel {
                                text: "Issued by"
                            }
                            DetailValue {
                                text: (root.certificate.issuer !== undefined ? root.certificate.issuer : "") + (root.certificate.selfSigned ? " (self-signed)" : "")
                            }

                            DetailLabel {
                                text: "Valid from"
                            }
                            DetailValue {
                                faulty: root.certificate.rejection === "notyetvalid"
                                text: root._date(root.certificate.validFrom) + (faulty ? " · not yet valid" : "")
                            }

                            DetailLabel {
                                text: "Valid until"
                            }
                            DetailValue {
                                faulty: root.certificate.rejection === "expired"
                                text: root._date(root.certificate.validUntil) + (faulty ? " · expired" : "")
                            }
                        }
                    }
                }
            }
        }

        // -- Footer
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: Theme.colorDivider
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            Layout.topMargin: 12
            Layout.bottomMargin: 12
            spacing: 8

            Text {
                visible: root._kind === "refused"
                Layout.fillWidth: true
                text: "Kemai never connects with an invalid certificate."
                color: Theme.colorTextSecondary
                font.pixelSize: Theme.fontSizeSmall
                wrapMode: Text.Wrap
            }

            Item {
                visible: root._kind !== "refused"
                Layout.fillWidth: true
            }

            DialogButton {
                id: cancelButton
                Layout.minimumWidth: 90
                variant: "primary"
                text: root._kind === "refused" ? "Close" : "Cancel"
                onClicked: root.close()
                Keys.onReturnPressed: root.close()
                Keys.onEnterPressed: root.close()
            }

            DialogButton {
                visible: root._kind !== "refused"
                variant: "ghost"
                tone: root._kind === "changed" ? "warning" : ""
                iconName: "shield"
                text: root._kind === "changed" ? "Trust new certificate" : "Trust and connect"
                onClicked: {
                    root.close()
                    root.trustRequested()
                }
            }
        }
    }

    component DetailLabel: Text {
        Layout.preferredWidth: 100
        color: Theme.colorTextSecondary
        font.pixelSize: Theme.fontSizeSmall
    }

    component DetailValue: Text {
        property bool faulty: false
        Layout.fillWidth: true
        color: faulty ? Theme.colorAccentRed : Theme.colorTextPrimary
        font.pixelSize: Theme.fontSizeSmall
        font.weight: faulty ? Font.DemiBold : Font.Normal
        font.features: Theme.fontFeaturesTabular
        wrapMode: Text.WrapAnywhere
    }
}
