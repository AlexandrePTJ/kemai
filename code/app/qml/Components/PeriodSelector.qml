// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kemai

import "HistoryPeriods.js" as HistoryPeriods

// Title of the history list: the period as a menu button, then its dates
GridLayout {
    id: root

    // The period named (the one being loaded, else the one shown) and its dates
    property string periodKey: "week"
    property date firstDate
    property date lastDate
    // A period is loading: spinner in place of the chevron, after a short delay
    property bool loading: false
    // Narrow window: the dates go under the name
    property bool narrow: false
    // key -> {first, last}, for the dates of the menu rows
    property var rangeOf: key => ({})

    signal chosen(string key)

    columns: root.narrow ? 1 : 2
    rowSpacing: 1
    columnSpacing: 8

    property bool _spinning: false

    onLoadingChanged: {
        if (root.loading)
            spinDelay.restart()
        else {
            spinDelay.stop()
            root._spinning = false
        }
    }

    Timer {
        id: spinDelay
        interval: 150
        onTriggered: root._spinning = root.loading
    }

    AbstractButton {
        id: periodButton

        Layout.preferredHeight: 28
        Layout.leftMargin: -10
        leftPadding: 10
        rightPadding: 8
        focusPolicy: Qt.TabFocus
        onClicked: periodMenu.opened ? periodMenu.close() : periodMenu.open()

        HoverHandler {
            cursorShape: Qt.PointingHandCursor
        }

        background: Rectangle {
            radius: Theme.radiusMedium
            readonly property bool lit: periodButton.hovered || periodMenu.opened
            color: lit ? Theme.colorSurface : "transparent"
            border.width: 1
            border.color: lit || periodButton.visualFocus ? Theme.colorBorderSubtle : "transparent"
        }

        contentItem: RowLayout {
            spacing: 6

            Text {
                text: HistoryPeriods.name(root.periodKey)
                color: Theme.colorTextPrimary
                font.pixelSize: Theme.fontSizeNormal
                font.weight: Font.DemiBold
            }

            Item {
                Layout.preferredWidth: 14
                Layout.preferredHeight: 14

                Spinner {
                    visible: root._spinning
                    anchors.centerIn: parent
                    size: 12
                    color: Theme.colorTextPrimary
                    trackColor: Theme.colorTextDim
                }

                Icon {
                    visible: !root._spinning
                    anchors.centerIn: parent
                    name: "chevron-down"
                    size: 14
                    color: Theme.colorTextSecondary
                }
            }
        }

        Popup {
            id: periodMenu

            // Row under the keyboard (or the mouse)
            property int highlighted: 0

            y: periodButton.height + 2
            width: 250
            padding: 4
            focus: true
            onAboutToShow: periodMenu.highlighted = Math.max(0, HistoryPeriods.keys.indexOf(root.periodKey))

            background: Rectangle {
                radius: Theme.radiusMedium
                color: Theme.colorBackgroundDark
                border.width: 1
                border.color: Theme.colorBorderSubtle
            }

            contentItem: ColumnLayout {
                spacing: 0
                focus: true

                Keys.onUpPressed: periodMenu.highlighted = (periodMenu.highlighted + HistoryPeriods.keys.length - 1) % HistoryPeriods.keys.length
                Keys.onDownPressed: periodMenu.highlighted = (periodMenu.highlighted + 1) % HistoryPeriods.keys.length
                Keys.onReturnPressed: periodMenu.choose(HistoryPeriods.keys[periodMenu.highlighted])
                Keys.onEnterPressed: periodMenu.choose(HistoryPeriods.keys[periodMenu.highlighted])

                Repeater {
                    model: HistoryPeriods.keys

                    delegate: AbstractButton {
                        id: periodRow

                        required property string modelData
                        required property int index

                        readonly property bool current: periodRow.modelData === root.periodKey
                        readonly property var range: root.rangeOf(periodRow.modelData)

                        Layout.fillWidth: true
                        topPadding: 8
                        bottomPadding: 8
                        leftPadding: 10
                        rightPadding: 10
                        focusPolicy: Qt.NoFocus
                        onHoveredChanged: if (periodRow.hovered)
                            periodMenu.highlighted = periodRow.index
                        onClicked: periodMenu.choose(periodRow.modelData)

                        HoverHandler {
                            cursorShape: Qt.PointingHandCursor
                        }

                        background: Rectangle {
                            radius: Theme.radiusSmall
                            color: periodMenu.highlighted === periodRow.index ? Theme.colorSurface : "transparent"
                        }

                        contentItem: RowLayout {
                            spacing: 8

                            Item {
                                Layout.preferredWidth: 14
                                Layout.preferredHeight: 14

                                Icon {
                                    visible: periodRow.current
                                    anchors.centerIn: parent
                                    name: "check"
                                    size: 13
                                    color: Theme.colorAccentGreen
                                }
                            }

                            Text {
                                Layout.fillWidth: true
                                text: HistoryPeriods.name(periodRow.modelData)
                                color: Theme.colorTextPrimary
                                font.pixelSize: Theme.fontSizeMedium
                                font.weight: periodRow.current ? Font.DemiBold : Font.Normal
                            }

                            Text {
                                text: HistoryPeriods.dates(periodRow.range.first, periodRow.range.last, Qt.locale())
                                color: Theme.colorTextSecondary
                                font.pixelSize: Theme.fontSizeXSmall
                                font.features: Theme.fontFeaturesTabular
                            }
                        }
                    }
                }
            }

            function choose(key) {
                periodMenu.close()
                if (key !== root.periodKey)
                    root.chosen(key)
            }
        }
    }

    Text {
        text: HistoryPeriods.dates(root.firstDate, root.lastDate, Qt.locale())
        color: Theme.colorTextSecondary
        font.pixelSize: Theme.fontSizeSmall
        font.features: Theme.fontFeaturesTabular
    }
}
