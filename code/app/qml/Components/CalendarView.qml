// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kemai

Item {
    id: root

    property int selectedYear
    property int selectedMonth
    property int selectedDay

    signal dateClicked(int year, int month, int day)

    property int _viewYear: selectedYear
    property int _viewMonth: selectedMonth

    onSelectedYearChanged: _viewYear = selectedYear
    onSelectedMonthChanged: _viewMonth = selectedMonth

    function _daysInMonth(y, m) { return new Date(y, m + 1, 0).getDate() }
    function _monthLabel(y, m) {
        const names = ["January", "February", "March", "April", "May", "June",
                       "July", "August", "September", "October", "November", "December"]
        return names[m] + " " + y
    }
    function _prevMonth() {
        if (_viewMonth === 0) { _viewMonth = 11; _viewYear-- } else { _viewMonth-- }
    }
    function _nextMonth() {
        if (_viewMonth === 11) { _viewMonth = 0; _viewYear++ } else { _viewMonth++ }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: Theme.spacingSmall

        // Month nav
        RowLayout {
            Layout.fillWidth: true

            AbstractButton {
                id: prevBtn
                implicitWidth: 28
                implicitHeight: 28
                contentItem: Item {
                    Icon {
                        anchors.centerIn: parent
                        name: "chevron-left"
                        size: 16
                        color: Theme.colorTextPrimary
                    }
                }
                background: Rectangle {
                    color: prevBtn.hovered ? Theme.colorSurface : "transparent"
                    radius: Theme.radiusSmall
                }
                onClicked: root._prevMonth()
            }

            Item { Layout.fillWidth: true }

            Text {
                text: root._monthLabel(root._viewYear, root._viewMonth)
                color: Theme.colorTextPrimary
                font.pixelSize: Theme.fontSizeNormal
                font.bold: true
            }

            Item { Layout.fillWidth: true }

            AbstractButton {
                id: nextBtn
                implicitWidth: 28
                implicitHeight: 28
                contentItem: Item {
                    Icon {
                        anchors.centerIn: parent
                        name: "chevron-right"
                        size: 16
                        color: Theme.colorTextPrimary
                    }
                }
                background: Rectangle {
                    color: nextBtn.hovered ? Theme.colorSurface : "transparent"
                    radius: Theme.radiusSmall
                }
                onClicked: root._nextMonth()
            }
        }

        // Weekday header
        Row {
            Layout.fillWidth: true
            Layout.topMargin: Theme.spacingSmall

            Repeater {
                model: ["SU", "MO", "TU", "WE", "TH", "FR", "SA"]
                delegate: Item {
                    required property string modelData
                    width: Math.floor(parent.width / 7)
                    height: 24

                    Text {
                        anchors.centerIn: parent
                        text: parent.modelData
                        color: Theme.colorTextSecondary
                        font.pixelSize: Theme.fontSizeXSmall
                        font.bold: true
                    }
                }
            }
        }

        // Day grid
        Grid {
            id: dayGrid
            Layout.fillWidth: true
            Layout.fillHeight: true
            rows: 6
            columns: 7

            readonly property int firstDayWeekday: new Date(root._viewYear, root._viewMonth, 1).getDay()
            readonly property int daysInThisMonth: root._daysInMonth(root._viewYear, root._viewMonth)
            readonly property int daysInPrevMonth: root._daysInMonth(
                root._viewMonth === 0 ? root._viewYear - 1 : root._viewYear,
                root._viewMonth === 0 ? 11 : root._viewMonth - 1)
            readonly property int cellWidth: Math.floor(width / 7)
            readonly property int cellHeight: Math.floor(height / 6)

            Repeater {
                model: 42
                delegate: Item {
                    id: dayCell
                    required property int index

                    width: dayGrid.cellWidth
                    height: dayGrid.cellHeight

                    readonly property int dayNumber: dayCell.index - dayGrid.firstDayWeekday + 1
                    readonly property bool inCurrentMonth: dayCell.dayNumber >= 1 && dayCell.dayNumber <= dayGrid.daysInThisMonth
                    readonly property int displayDay: dayCell.inCurrentMonth ? dayCell.dayNumber
                        : (dayCell.dayNumber < 1 ? dayGrid.daysInPrevMonth + dayCell.dayNumber
                                                 : dayCell.dayNumber - dayGrid.daysInThisMonth)
                    readonly property bool isSelected: dayCell.inCurrentMonth
                        && dayCell.displayDay === root.selectedDay
                        && root._viewYear === root.selectedYear
                        && root._viewMonth === root.selectedMonth

                    Rectangle {
                        anchors.centerIn: parent
                        width: 30
                        height: 30
                        radius: 15
                        color: dayCell.isSelected ? Theme.colorAccentGreen
                            : (cellMouse.containsMouse && dayCell.inCurrentMonth ? Theme.colorSurface : "transparent")

                        Text {
                            anchors.centerIn: parent
                            text: dayCell.displayDay
                            color: dayCell.isSelected ? Theme.colorTextDarkPrimary
                                : (dayCell.inCurrentMonth ? Theme.colorTextPrimary : Theme.colorTextSecondary)
                            font.pixelSize: Theme.fontSizeNormal
                            font.bold: dayCell.isSelected
                        }
                    }

                    MouseArea {
                        id: cellMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (dayCell.inCurrentMonth) {
                                root.dateClicked(root._viewYear, root._viewMonth, dayCell.displayDay)
                            } else if (dayCell.dayNumber < 1) {
                                const newM = root._viewMonth === 0 ? 11 : root._viewMonth - 1
                                const newY = root._viewMonth === 0 ? root._viewYear - 1 : root._viewYear
                                root.dateClicked(newY, newM, dayCell.displayDay)
                            } else {
                                const newM = root._viewMonth === 11 ? 0 : root._viewMonth + 1
                                const newY = root._viewMonth === 11 ? root._viewYear + 1 : root._viewYear
                                root.dateClicked(newY, newM, dayCell.displayDay)
                            }
                        }
                    }
                }
            }
        }
    }
}
