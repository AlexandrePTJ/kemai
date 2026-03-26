// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    property string timerText: "00:00:00"
    property string entryDescription: ""
    property bool isRunning: false

    signal stopClicked()
    signal startClicked()

    implicitHeight: 56
    color: Theme.colorSurfaceLight

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 18
        anchors.rightMargin: 18
        spacing: 12

        // Timer display
        Text {
            text: root.timerText
            color: root.isRunning ? Theme.colorAccentGreen : Theme.colorTextSecondary
            font.family: Theme.fontFamilyMono
            font.pixelSize: 26
            font.bold: true
            Layout.minimumWidth: 110
        }

        KemaiTextField {
            Layout.fillWidth: true
            text: root.entryDescription
        }

        StartStopButton {
            implicitHeight: 46
            implicitWidth: 80

            isStarted: root.isRunning
            onClicked: root.isRunning ? root.stopClicked() : root.startClicked()
        }
    }
}
