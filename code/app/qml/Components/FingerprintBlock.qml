// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// SHA-256 fingerprint of a certificate: caps label, Copy (copies "AB:CD:…"), and the 32 bytes on two
// selectable lines of four groups
Rectangle {
    id: root

    property string label: "SHA-256 fingerprint"
    // "AB:CD:…"
    property string sha256: ""
    // Old fingerprint of a changed certificate: muted, struck through
    property bool superseded: false
    // New fingerprint of a changed certificate: amber label and border
    property bool highlighted: false
    property bool copyable: true

    property bool _copied: false

    // "AB CD EF 01  23 45 …": 16 bytes per line, groups of 4
    function _line(index) {
        const bytes = root.sha256.split(":").slice(index * 16, index * 16 + 16)
        const groups = []
        for (let g = 0; g < bytes.length; g += 4)
            groups.push(bytes.slice(g, g + 4).join(" "))
        return groups.join("   ")
    }

    implicitHeight: blockColumn.implicitHeight + 20
    radius: Theme.radiusMedium
    color: Theme.colorBackgroundDark
    border.width: 1
    border.color: root.highlighted ? Theme.colorWarningLine : Theme.colorBorderSubtle

    // Clipboard access through a hidden editor
    TextEdit {
        id: clipboardHelper
        visible: false
    }

    Timer {
        id: copiedTimer
        interval: 2000
        onTriggered: root._copied = false
    }

    ColumnLayout {
        id: blockColumn
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 10
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        spacing: 6

        RowLayout {
            Layout.fillWidth: true
            spacing: 6

            Text {
                Layout.fillWidth: true
                text: root.label.toUpperCase()
                color: root.highlighted ? Theme.colorWarning : Theme.colorTextLabel
                font.pixelSize: Theme.fontSizeXSmall
                font.weight: Font.DemiBold
                font.letterSpacing: 1.1
            }

            AbstractButton {
                id: copyButton
                visible: root.copyable
                focusPolicy: Qt.StrongFocus
                onClicked: {
                    clipboardHelper.text = root.sha256
                    clipboardHelper.selectAll()
                    clipboardHelper.copy()
                    root._copied = true
                    copiedTimer.restart()
                }

                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }

                background: Rectangle {
                    color: "transparent"
                    radius: Theme.radiusSmall
                    border.width: copyButton.visualFocus ? 1 : 0
                    border.color: Theme.colorAccentGreenDim
                }

                contentItem: RowLayout {
                    spacing: 4

                    Icon {
                        name: root._copied ? "check" : "copy"
                        size: 12
                        color: root._copied ? Theme.colorAccentGreen : Theme.colorTextSecondary
                    }

                    Text {
                        text: root._copied ? "Copied" : "Copy"
                        color: root._copied ? Theme.colorAccentGreen : Theme.colorTextSecondary
                        font.pixelSize: Theme.fontSizeXSmall
                    }
                }
            }
        }

        Repeater {
            model: 2

            delegate: TextEdit {
                id: fingerprintLine

                required property int index

                Layout.fillWidth: true
                readOnly: true
                selectByMouse: true
                text: root._line(fingerprintLine.index)
                color: root.superseded ? Theme.colorTextSecondary : Theme.colorTextPrimary
                selectionColor: Theme.colorAccentGreenDim
                font.family: Theme.fontFamilyMono
                font.pixelSize: Theme.fontSizeSmall
                font.letterSpacing: 0.3
                font.strikeout: root.superseded
            }
        }
    }
}
