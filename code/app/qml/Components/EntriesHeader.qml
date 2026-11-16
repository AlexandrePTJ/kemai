// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kemai

Item {
    id: root

    property string todayDuration: "0h 00m"
    property string weekDuration: "0h 00m"
    property bool refreshing: false
    // Narrow window: "WK" for the week total, tighter spacing
    property bool narrow: false

    // Extra controls at the far right, after a divider (e.g. the account menu)
    default property alias trailing: trailingRow.data
    // At the left: the list's title (e.g. the history period)
    property alias leading: leadingRow.data
    // Right of the spacer, before the totals (e.g. a filter field)
    property alias middle: middleRow.data
    // The Today / Week totals and the refresh button
    property bool statsVisible: true
    // The totals alone (a narrow header gives their room to an open filter)
    property bool totalsVisible: true
    // `middle` takes all the free width (e.g. an open filter in a narrow header)
    property bool middleFills: false

    signal refreshClicked

    implicitHeight: 48

    component Stat: Item {
        id: stat

        property string label
        property string value

        implicitWidth: 64
        implicitHeight: 30

        Text {
            id: statLabel
            anchors.horizontalCenter: parent.horizontalCenter
            // The letter spacing also follows the last letter: centre the letters, not the box
            anchors.horizontalCenterOffset: 0.6
            y: 0
            height: 12
            verticalAlignment: Text.AlignVCenter
            text: stat.label.toUpperCase()
            color: Theme.colorTextLabel
            font.pixelSize: Theme.fontSizeTiny
            font.weight: Font.DemiBold
            font.letterSpacing: 1.2
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: statLabel.bottom
            height: 18
            verticalAlignment: Text.AlignVCenter
            text: stat.value
            color: Theme.colorAccentGreen
            font.pixelSize: Theme.fontSizeMedium
            font.weight: Font.DemiBold
            font.features: Theme.fontFeaturesTabular
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: root.narrow ? 10 : 14

        RowLayout {
            id: leadingRow
            spacing: 8
            Layout.fillWidth: false
            Layout.minimumWidth: 0
        }

        Item {
            visible: !root.middleFills
            Layout.fillWidth: true
        }

        // Takes free width up to what its items allow (a filter field shrinks rather than push the account menu out)
        RowLayout {
            id: middleRow
            spacing: 8
            Layout.fillWidth: true
        }

        // Totals: always today and the current week, whatever the list shows
        RowLayout {
            visible: root.statsVisible && root.totalsVisible
            spacing: 4
            Layout.fillWidth: false
            Layout.fillHeight: false
            Layout.preferredHeight: 30

            Rectangle {
                Layout.preferredWidth: 1
                Layout.fillHeight: true
                Layout.rightMargin: 6
                color: Theme.colorDivider
            }

            Stat {
                label: "Today"
                value: root.todayDuration
            }

            Stat {
                id: weekStat
                label: root.narrow ? "Wk" : "Week"
                value: root.weekDuration

                HoverHandler {
                    id: weekHover
                }

                ToolTip.visible: weekHover.hovered
                ToolTip.delay: 500
                ToolTip.text: "Current week, Mon – Sun, whatever the period shown"
            }
        }

        // Refresh (projects/activities cache + timesheets)
        AbstractButton {
            id: refreshButton
            visible: root.statsVisible
            enabled: !root.refreshing
            Layout.preferredWidth: 30
            Layout.preferredHeight: 30
            focusPolicy: Qt.TabFocus
            onClicked: root.refreshClicked()

            ToolTip.visible: refreshButton.hovered
            ToolTip.delay: 500
            ToolTip.text: "Refresh"

            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }

            background: Rectangle {
                radius: Theme.radiusMedium
                color: refreshButton.hovered ? Theme.colorSurface : "transparent"
                border.width: 1
                border.color: refreshButton.visualFocus ? Theme.colorAccentGreenDim : Theme.colorBorderSubtle
            }

            contentItem: Item {
                Icon {
                    id: refreshIcon
                    anchors.centerIn: parent
                    name: "refresh-cw"
                    size: 14
                    color: Theme.colorTextSecondary

                    RotationAnimator on rotation {
                        running: root.refreshing
                        from: 0
                        to: 360
                        duration: 1000
                        loops: Animation.Infinite
                        onStopped: refreshIcon.rotation = 0
                    }
                }
            }
        }

        Rectangle {
            visible: trailingRow.children.length > 0
            Layout.leftMargin: 2
            Layout.rightMargin: 2
            Layout.preferredWidth: 1
            Layout.preferredHeight: 20
            color: Theme.colorDivider
        }

        RowLayout {
            id: trailingRow
            spacing: 8
            Layout.fillWidth: false
        }
    }
}
