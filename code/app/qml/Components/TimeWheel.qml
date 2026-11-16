// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kemai

Item {
    id: root

    property int selectedHour
    property int selectedMinute
    property bool is24h: true

    signal timeClicked(int hour, int minute)
    signal modeChanged(bool is24h)

    function scrollToCurrent() {
        timeList.positionViewAtIndex(timeList.currentIndex, ListView.Center)
    }

    function _pad2(n) {
        return n < 10 ? "0" + n : "" + n
    }
    function _formatTime(h, mi) {
        if (is24h)
            return _pad2(h) + ":" + _pad2(mi)
        const period = h >= 12 ? "PM" : "AM"
        let h12 = h % 12
        if (h12 === 0)
            h12 = 12
        return h12 + ":" + _pad2(mi) + " " + period
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: Theme.spacingSmall

        // Header: TIME label + 24h/12h toggle
        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spacingSmall

            Text {
                text: "TIME"
                color: Theme.colorTextSecondary
                font.pixelSize: Theme.fontSizeXSmall
                font.bold: true
            }

            Item {
                Layout.fillWidth: true
            }

            Rectangle {
                implicitWidth: 80
                implicitHeight: 24
                color: Theme.colorSurfaceDark
                radius: Theme.radiusSmall
                border.color: Theme.colorBorder
                border.width: 1

                Row {
                    anchors.fill: parent
                    anchors.margins: 2
                    spacing: 0

                    AbstractButton {
                        id: btn24
                        width: parent.width / 2
                        height: parent.height
                        contentItem: Text {
                            text: "24h"
                            color: root.is24h ? Theme.colorTextPrimary : Theme.colorTextSecondary
                            font.pixelSize: Theme.fontSizeXSmall
                            font.bold: root.is24h
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            color: root.is24h ? Theme.colorBackground : "transparent"
                            radius: Theme.radiusSmall - 1
                        }
                        onClicked: root.modeChanged(true)
                    }

                    AbstractButton {
                        id: btn12
                        width: parent.width / 2
                        height: parent.height
                        contentItem: Text {
                            text: "12h"
                            color: !root.is24h ? Theme.colorTextPrimary : Theme.colorTextSecondary
                            font.pixelSize: Theme.fontSizeXSmall
                            font.bold: !root.is24h
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            color: !root.is24h ? Theme.colorBackground : "transparent"
                            radius: Theme.radiusSmall - 1
                        }
                        onClicked: root.modeChanged(false)
                    }
                }
            }
        }

        // Time list
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Theme.colorInputBackground
            radius: Theme.radiusMedium
            clip: true

            ListView {
                id: timeList
                anchors.fill: parent
                anchors.margins: 4
                model: 24 * 60
                clip: true
                currentIndex: root.selectedHour * 60 + root.selectedMinute
                highlightFollowsCurrentItem: false
                boundsBehavior: Flickable.StopAtBounds

                delegate: Item {
                    id: timeRow
                    required property int index

                    width: timeList.width
                    height: 28

                    readonly property int rowHour: Math.floor(timeRow.index / 60)
                    readonly property int rowMinute: timeRow.index % 60
                    readonly property bool isSelected: timeRow.rowHour === root.selectedHour && timeRow.rowMinute === root.selectedMinute

                    Rectangle {
                        anchors.fill: parent
                        anchors.leftMargin: 6
                        anchors.rightMargin: 6
                        color: timeRow.isSelected ? Theme.colorAccentGreen : (timeHover.hovered ? Theme.colorSurface : "transparent")
                        radius: Theme.radiusSmall

                        Text {
                            anchors.centerIn: parent
                            text: root._formatTime(timeRow.rowHour, timeRow.rowMinute)
                            color: timeRow.isSelected ? Theme.colorTextDarkPrimary : Theme.colorTextSecondary
                            font.pixelSize: Theme.fontSizeNormal
                            font.bold: timeRow.isSelected
                            font.features: Theme.fontFeaturesTabular
                        }
                    }

                    HoverHandler {
                        id: timeHover
                        cursorShape: Qt.PointingHandCursor
                    }

                    TapHandler {

                        gesturePolicy: TapHandler.ReleaseWithinBounds
                        onTapped: root.timeClicked(timeRow.rowHour, timeRow.rowMinute)
                    }
                }
            }
        }
    }
}
