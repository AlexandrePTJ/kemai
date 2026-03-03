// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    
    RowLayout {
            anchors.fill: parent
            spacing: 10

            Text {
                text: root.formatDuration(root.activeSeconds)
                color: Theme.colorAccentGreen
                font.pixelSize: Theme.fontSizeTimer
                font.bold: true
                font.family: Theme.fontFamilyMono
                Layout.alignment: Qt.AlignVCenter
            }

            TextField {
                Layout.fillWidth: true

                text: "Kemai Refactoring - Development"
                color: Theme.colorTextPrimary
                font.pixelSize: Theme.fontSizeNormal
                background: Rectangle {
                    color: Theme.colorSurface
                    radius: 4
                }
                placeholderTextColor: Theme.colorTextSecondary
                leftPadding: 10
                rightPadding: 10
            }

            Button {
                text: "■ Stop"
                font.pixelSize: Theme.fontSizeMedium
                padding: 10
                background: Rectangle {
                    color: parent.down ? Theme.colorButtonStopDown : Theme.colorButtonStop
                    radius: 4
                }
                contentItem: Text {
                    text: parent.text
                    color: Theme.colorTextPrimary
                    font: parent.font
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
}