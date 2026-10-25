// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kemai

Popup {
    id: root

    property string title: "Edit Time"
    property date initialDateTime: new Date()

    readonly property date selectedDateTime: new Date(_year, _month, _day, _hour, _minute, 0, 0)

    signal confirmed(date dateTime)

    property int _year: 2000
    property int _month: 0
    property int _day: 1
    property int _hour: 0
    property int _minute: 0
    property bool _is24h: true

    modal: true
    width: 560
    height: 520
    x: parent ? (parent.width - width) / 2 : 0
    y: parent ? (parent.height - height) / 2 : 0
    padding: 0

    background: Rectangle {
        color: Theme.colorBackground
        border.color: Theme.colorBorder
        border.width: 1
        radius: Theme.radiusLarge
    }

    function _pad2(n) { return n < 10 ? "0" + n : "" + n }
    function _formatDate(y, m, d) { return y + "-" + _pad2(m + 1) + "-" + _pad2(d) }
    function _formatTime(h, mi) {
        if (_is24h) return _pad2(h) + ":" + _pad2(mi)
        const period = h >= 12 ? "PM" : "AM"
        let h12 = h % 12
        if (h12 === 0) h12 = 12
        return h12 + ":" + _pad2(mi) + " " + period
    }
    function _shortDateLabel(y, m, d) {
        return new Date(y, m, d).toLocaleDateString(Qt.locale(), "ddd, MMM d")
    }
    function _setNow() {
        const now = new Date()
        _year = now.getFullYear()
        _month = now.getMonth()
        _day = now.getDate()
        _hour = now.getHours()
        _minute = now.getMinutes()
        timeWheel.scrollToCurrent()
    }

    onAboutToShow: {
        const dt = isNaN(initialDateTime.getTime()) ? new Date() : initialDateTime
        _year = dt.getFullYear()
        _month = dt.getMonth()
        _day = dt.getDate()
        _hour = dt.getHours()
        _minute = dt.getMinutes()
    }
    onOpened: timeWheel.scrollToCurrent()

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // -- Draggable title bar
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 44
            color: Theme.colorBackgroundDark
            radius: Theme.radiusLarge

            // Square off the bottom corners of the rounded header
            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: Theme.radiusLarge
                color: parent.color
            }

            Text {
                anchors.left: parent.left
                anchors.leftMargin: Theme.spacingLarge
                anchors.verticalCenter: parent.verticalCenter
                text: root.title
                color: Theme.colorTextPrimary
                font.pixelSize: Theme.fontSizeNormal
                font.bold: true
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.SizeAllCursor
                property real pressX: 0
                property real pressY: 0
                onPressed: (mouse) => { pressX = mouse.x; pressY = mouse.y }
                onPositionChanged: (mouse) => {
                    if (pressed) {
                        root.x += mouse.x - pressX
                        root.y += mouse.y - pressY
                    }
                }
            }
        }

        // -- Editable date and time fields
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: Theme.spacingLarge
            Layout.rightMargin: Theme.spacingLarge
            Layout.topMargin: Theme.spacingMedium
            spacing: Theme.spacingMedium

            KemaiTextField {
                id: dateField
                Layout.fillWidth: true
                inputMask: "9999-99-99"
                font.family: Theme.fontFamilyMono
                horizontalAlignment: TextInput.AlignHCenter

                Binding on text {
                    value: root._formatDate(root._year, root._month, root._day)
                    when: !dateField.activeFocus
                }

                onEditingFinished: {
                    const parts = text.split("-")
                    if (parts.length !== 3) return
                    const y = parseInt(parts[0])
                    const m = parseInt(parts[1]) - 1
                    const d = parseInt(parts[2])
                    if (isNaN(y) || isNaN(m) || isNaN(d)) return
                    if (m < 0 || m > 11) return
                    const maxDay = new Date(y, m + 1, 0).getDate()
                    if (d < 1 || d > maxDay) return
                    root._year = y
                    root._month = m
                    root._day = d
                }
            }

            KemaiTextField {
                id: timeField
                Layout.fillWidth: true
                inputMask: "99:99"
                font.family: Theme.fontFamilyMono
                horizontalAlignment: TextInput.AlignHCenter

                Binding on text {
                    value: root._formatTime(root._hour, root._minute)
                    when: !timeField.activeFocus
                }

                // Switch to 24h-shaped text on focus to match the input mask;
                // the binding restores the formatted (12h or 24h) text on blur.
                onActiveFocusChanged: {
                    if (activeFocus)
                        text = root._pad2(root._hour) + ":" + root._pad2(root._minute)
                }

                onEditingFinished: {
                    const parts = text.split(":")
                    if (parts.length !== 2) return
                    const h = parseInt(parts[0])
                    const mi = parseInt(parts[1])
                    if (isNaN(h) || isNaN(mi)) return
                    if (h < 0 || h > 23 || mi < 0 || mi > 59) return
                    root._hour = h
                    root._minute = mi
                    timeWheel.scrollToCurrent()
                }
            }
        }

        // -- Calendar + time wheel
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: Theme.spacingLarge
            Layout.rightMargin: Theme.spacingLarge
            Layout.topMargin: Theme.spacingMedium
            spacing: Theme.spacingLarge

            CalendarView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                selectedYear: root._year
                selectedMonth: root._month
                selectedDay: root._day
                onDateClicked: (year, month, day) => {
                    root._year = year
                    root._month = month
                    root._day = day
                }
            }

            TimeWheel {
                id: timeWheel
                Layout.preferredWidth: 170
                Layout.fillHeight: true
                selectedHour: root._hour
                selectedMinute: root._minute
                is24h: root._is24h
                onTimeClicked: (hour, minute) => {
                    root._hour = hour
                    root._minute = minute
                }
                onModeChanged: (is24h) => root._is24h = is24h
            }
        }

        // -- Summary + minute shortcuts
        Rectangle {
            Layout.fillWidth: true
            Layout.topMargin: Theme.spacingMedium
            implicitHeight: 48
            color: Theme.colorBackgroundDark

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: Theme.spacingLarge
                anchors.rightMargin: Theme.spacingLarge
                spacing: Theme.spacingSmall

                Text {
                    text: root._shortDateLabel(root._year, root._month, root._day)
                          + " — " + root._formatTime(root._hour, root._minute)
                    color: Theme.colorAccentGreen
                    font.pixelSize: Theme.fontSizeSmall
                    font.family: Theme.fontFamilyMono
                }

                Item { Layout.fillWidth: true }

                Repeater {
                    model: [0, 15, 30, 45]
                    delegate: SecondaryButton {
                        required property int modelData
                        text: ":" + (modelData < 10 ? "0" + modelData : modelData)
                        Layout.preferredHeight: 28
                        onClicked: {
                            root._minute = modelData
                            timeWheel.scrollToCurrent()
                        }
                    }
                }

                SecondaryButton {
                    text: "Now"
                    Layout.preferredHeight: 28
                    onClicked: root._setNow()
                }
            }
        }

        // -- Footer
        RowLayout {
            Layout.fillWidth: true
            Layout.margins: Theme.spacingMedium
            spacing: Theme.spacingSmall

            Item { Layout.fillWidth: true }

            SecondaryButton {
                text: "Cancel"
                textSize: Theme.fontSizeSmall
                Layout.preferredHeight: 32
                Layout.preferredWidth: 80
                onClicked: root.close()
            }

            PrimaryEditButton {
                text: "Confirm"
                Layout.preferredHeight: 32
                Layout.preferredWidth: 80
                onClicked: {
                    root.confirmed(root.selectedDateTime)
                    root.close()
                }
            }
        }
    }
}
