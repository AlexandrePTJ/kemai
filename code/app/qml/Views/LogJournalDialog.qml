// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kemai

// What Kemai logged since it started, to understand a problem and hand it to a maintainer: level filter, copy
// of the listed entries (or of the selected rows with Ctrl+C), log files folder. Read-only; the filter, Wrap and
// Follow are kept until Kemai quits.
Popup {
    id: root

    required property LogJournal journal

    // LogEntryModel.Level: 0 debug, 1 info, 2 warning, 3 error
    property int _minLevel: 0
    property bool _wrap: false
    property bool _follow: true
    // Following and the view at the bottom; false once the user scrolls up
    property bool _pinned: true
    property int _unseen: 0
    // Selected rows: from the anchor to the cursor (listed row indexes), -1 for none
    property int _anchor: -1
    property int _cursor: -1
    readonly property int _selectionFirst: Math.min(root._anchor, root._cursor)
    readonly property int _selectionLast: Math.max(root._anchor, root._cursor)
    property int _copiedCount: -1
    // Newest row while following: highlighted, fading out
    property int _freshRow: -1
    property int _freshStamp: 0

    readonly property var _levels: [
        {
            badge: "DBG",
            color: Theme.colorTextDim,
            background: "transparent",
            border: Theme.colorBorderSubtle
        },
        {
            badge: "INF",
            color: Theme.colorInfo,
            background: Theme.colorInfoSoft,
            border: Theme.colorInfoLine
        },
        {
            badge: "WRN",
            color: Theme.colorWarning,
            background: Theme.colorWarningSoft,
            border: Theme.colorWarningLine
        },
        {
            badge: "ERR",
            color: Theme.colorAccentRed,
            background: Theme.colorAccentRedSoft,
            border: Theme.colorAccentRedLine
        }
    ]

    function _number(n) {
        return Number(n).toLocaleString(Qt.locale(), "f", 0)
    }

    function _entries(n) {
        return n === 1 ? "1 entry" : root._number(n) + " entries"
    }

    function _clearSelection() {
        root._anchor = -1
        root._cursor = -1
    }

    function _select(row, extend) {
        if (extend && root._anchor >= 0) {
            root._cursor = row
        } else {
            root._anchor = row
            root._cursor = row
        }
    }

    function _scrollToEnd() {
        entryList.autoScrolling = true
        entryList.positionViewAtEnd()
        entryList.autoScrolling = false
        root._pinned = true
        root._unseen = 0
    }

    function _copyListed() {
        root.journal.copyToClipboard(root.journal.reportHeader() + "\n" + listed.text(0, listed.count - 1))
        root._copiedCount = listed.count
        copiedTimer.restart()
    }

    parent: Overlay.overlay
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape
    width: Math.min(740, parent ? parent.width - 40 : 740)
    height: Math.min(540, parent ? parent.height - 40 : 540)
    x: parent ? (parent.width - width) / 2 : 0
    y: parent ? (parent.height - height) / 2 : 0
    padding: 0

    onOpened: {
        copyButton.forceActiveFocus(Qt.TabFocusReason)
        Qt.callLater(root._scrollToEnd)
    }
    onClosed: root._clearSelection()

    on_MinLevelChanged: {
        root._clearSelection()
        if (root._follow)
            Qt.callLater(root._scrollToEnd)
    }

    LogFilterProxyModel {
        id: listed
        sourceModel: root.journal.entries
        minLevel: root._minLevel

        onRowsInserted: (parent, first, last) => {
            if (!root.opened || !root._follow)
                return
            if (root._pinned) {
                root._freshRow = last
                root._freshStamp++
                Qt.callLater(root._scrollToEnd)
            } else {
                root._unseen += last - first + 1
            }
        }
        // The oldest entries go once the journal is full: the selected rows move up with them
        onRowsRemoved: (parent, first, last) => {
            const removed = last - first + 1
            root._freshRow -= removed
            if (root._anchor < 0)
                return
            if (first !== 0 || root._selectionLast < removed) {
                root._clearSelection()
                return
            }
            root._anchor = Math.max(0, root._anchor - removed)
            root._cursor = Math.max(0, root._cursor - removed)
        }
    }

    Timer {
        id: copiedTimer
        interval: 2000
        onTriggered: root._copiedCount = -1
    }

    Shortcut {
        sequences: [StandardKey.Copy]
        enabled: root.opened && root._selectionFirst >= 0
        onActivated: root.journal.copyToClipboard(listed.text(root._selectionFirst, root._selectionLast))
    }

    Shortcut {
        sequences: [StandardKey.SelectAll]
        enabled: root.opened && listed.count > 0
        onActivated: {
            root._anchor = 0
            root._cursor = listed.count - 1
        }
    }

    Overlay.modal: Rectangle {
        color: Theme.colorBackdrop
    }

    background: Rectangle {
        color: Theme.colorBackground
        border.color: Theme.colorBorderSubtle
        border.width: 1
        radius: Theme.radiusDialog
    }

    component Divider: Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: 1
        color: Theme.colorDivider
    }

    component SmallToggle: RowLayout {
        id: smallToggle

        property alias checked: toggle.checked
        property string label

        spacing: 7

        SettingsToggle {
            id: toggle
            Layout.preferredWidth: 28
            Layout.preferredHeight: 16
        }

        Text {
            text: smallToggle.label
            color: Theme.colorTextSecondary
            font.pixelSize: Theme.fontSizeSmall

            TapHandler {
                gesturePolicy: TapHandler.ReleaseWithinBounds
                onTapped: toggle.toggle()
            }
        }
    }

    contentItem: ColumnLayout {
        spacing: 0

        // -- Header
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 16
            Layout.rightMargin: 12
            Layout.topMargin: 12
            Layout.bottomMargin: 12
            spacing: 10

            Icon {
                name: "list"
                size: 16
                color: Theme.colorTextSecondary
            }

            Text {
                text: "Log journal"
                color: Theme.colorTextPrimary
                font.pixelSize: Theme.fontSizeNormal
                font.weight: Font.DemiBold
            }

            Text {
                Layout.fillWidth: true
                text: "since Kemai started · " + root._entries(root.journal.count)
                color: Theme.colorTextSecondary
                font.pixelSize: Theme.fontSizeSmall
                elide: Text.ElideRight
            }

            AbstractButton {
                id: closeCross
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                focusPolicy: Qt.NoFocus
                onClicked: root.close()

                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }

                background: Rectangle {
                    radius: Theme.radiusSmall
                    color: closeCross.hovered ? Theme.colorSurface : "transparent"
                }

                contentItem: Item {
                    Icon {
                        anchors.centerIn: parent
                        name: "x"
                        size: 16
                        color: closeCross.hovered ? Theme.colorTextPrimary : Theme.colorTextSecondary
                    }
                }
            }
        }

        Divider {}

        // -- Toolbar
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 16
            Layout.rightMargin: 16
            Layout.topMargin: 10
            Layout.bottomMargin: 10
            spacing: 12

            SegmentedControl {
                Layout.preferredHeight: 28
                options: ["All", "Info +", "Warnings +", "Errors"]
                swatches: ["transparent", Theme.colorInfo, Theme.colorWarning, Theme.colorAccentRed]
                currentIndex: root._minLevel
                onActivated: index => root._minLevel = index
            }

            Text {
                textFormat: Text.StyledText
                text: "<font color=\"" + Theme.colorAccentRed + "\"><b>" + root._number(root.journal.errorCount) + "</b></font> " + (root.journal.errorCount === 1 ? "error" : "errors") + " · <font color=\"" + Theme.colorWarning + "\"><b>" + root._number(root.journal.warningCount) + "</b></font> " + (root.journal.warningCount === 1 ? "warning" : "warnings")
                color: Theme.colorTextSecondary
                font.pixelSize: Theme.fontSizeXSmall
            }

            Item {
                Layout.fillWidth: true
            }

            SmallToggle {
                label: "Wrap"
                checked: root._wrap
                onCheckedChanged: root._wrap = checked
            }

            SmallToggle {
                label: "Follow"
                checked: root._follow
                onCheckedChanged: {
                    root._follow = checked
                    root._unseen = 0
                    if (checked)
                        Qt.callLater(root._scrollToEnd)
                }
            }
        }

        Divider {}

        // -- Entries
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Theme.colorBackgroundDark
            clip: true

            ListView {
                id: entryList

                // True while Kemai scrolls the view itself: only the user's scrolling pauses following
                property bool autoScrolling: false

                anchors.fill: parent
                // Few entries sit at the bottom, newest last
                topMargin: Math.max(6, entryList.height - entryList.contentHeight - 6)
                bottomMargin: 6
                model: listed
                boundsBehavior: Flickable.StopAtBounds
                reuseItems: true

                onContentYChanged: {
                    if (entryList.autoScrolling || !root._follow)
                        return
                    const atEnd = entryList.contentY >= entryList.originY + entryList.contentHeight + entryList.bottomMargin - entryList.height - 2
                    root._pinned = atEnd
                    if (atEnd)
                        root._unseen = 0
                }

                WheelScroller {
                    flickable: entryList
                }

                ScrollBar.vertical: ScrollBar {}

                delegate: Rectangle {
                    id: row

                    required property int index
                    required property string time
                    required property int level
                    required property string message

                    readonly property bool selected: row.index >= root._selectionFirst && row.index <= root._selectionLast && root._selectionFirst >= 0
                    readonly property var style: root._levels[row.level] ?? root._levels[0]

                    width: entryList.width
                    implicitHeight: rowContent.implicitHeight + 8
                    color: row.selected ? Theme.colorAccentGreenSoft : row.level === 3 ? Theme.colorRowError : "transparent"

                    function _flashIfFresh() {
                        if (row.index === root._freshRow)
                            fade.restart()
                    }

                    Component.onCompleted: row._flashIfFresh()
                    ListView.onReused: row._flashIfFresh()

                    Connections {
                        target: root
                        function on_FreshStampChanged() {
                            row._flashIfFresh()
                        }
                    }

                    Rectangle {
                        id: freshMark
                        anchors.fill: parent
                        color: Theme.colorRowFresh
                        opacity: 0
                    }

                    NumberAnimation {
                        id: fade
                        target: freshMark
                        property: "opacity"
                        from: 1
                        to: 0
                        duration: 1000
                    }

                    Rectangle {
                        width: 2
                        height: parent.height
                        color: row.level === 3 ? Theme.colorAccentRed : row.level === 2 ? Theme.colorWarning : "transparent"
                    }

                    HoverHandler {
                        id: rowHover
                    }

                    TapHandler {
                        gesturePolicy: TapHandler.ReleaseWithinBounds
                        onTapped: root._select(row.index, (point.modifiers & Qt.ShiftModifier) !== 0)
                    }

                    ToolTip.visible: rowHover.hovered && messageText.truncated
                    ToolTip.delay: 600
                    ToolTip.text: row.message

                    RowLayout {
                        id: rowContent
                        x: 14
                        y: 4
                        width: row.width - 28
                        spacing: 10

                        Text {
                            Layout.alignment: Qt.AlignTop
                            text: row.time
                            color: Theme.colorTextSecondary
                            font.family: Theme.fontFamilyMono
                            font.pixelSize: Theme.fontSizeSmall
                            font.features: Theme.fontFeaturesTabular
                            lineHeightMode: Text.FixedHeight
                            lineHeight: 18
                        }

                        Rectangle {
                            Layout.alignment: Qt.AlignTop
                            Layout.topMargin: 1
                            Layout.preferredWidth: 34
                            Layout.preferredHeight: 16
                            radius: 3
                            color: row.style.background
                            border.width: 1
                            border.color: row.style.border

                            Text {
                                anchors.centerIn: parent
                                text: row.style.badge
                                color: row.style.color
                                font.pixelSize: Theme.fontSizeTiny
                                font.weight: Font.DemiBold
                                font.letterSpacing: 0.5
                            }
                        }

                        Text {
                            id: messageText
                            Layout.fillWidth: true
                            text: row.message
                            textFormat: Text.PlainText
                            color: row.level === 0 ? Theme.colorTextSecondary : Theme.colorTextPrimary
                            font.family: Theme.fontFamilyMono
                            font.pixelSize: Theme.fontSizeSmall
                            lineHeightMode: Text.FixedHeight
                            lineHeight: 18
                            wrapMode: root._wrap ? Text.WrapAnywhere : Text.NoWrap
                            elide: root._wrap ? Text.ElideNone : Text.ElideRight
                            maximumLineCount: root._wrap ? 1000 : 1
                        }
                    }
                }
            }

            // Nothing at this level
            ColumnLayout {
                visible: listed.count === 0
                anchors.centerIn: parent
                width: Math.min(parent.width - 40, 420)
                spacing: 8

                Icon {
                    Layout.alignment: Qt.AlignHCenter
                    name: "check"
                    size: 22
                    color: Theme.colorAccentGreen
                }

                Text {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    text: root._minLevel === 3 ? "No errors since Kemai started" : root._minLevel === 2 ? "No warnings or errors since Kemai started" : "No entries at this level"
                    color: Theme.colorTextPrimary
                    font.pixelSize: Theme.fontSizeMedium
                    font.weight: Font.DemiBold
                    wrapMode: Text.Wrap
                }

                Text {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    text: (root.journal.count === 1 ? "1 entry is" : root._number(root.journal.count) + " entries are") + " hidden by this filter."
                    color: Theme.colorTextSecondary
                    font.pixelSize: Theme.fontSizeSmall
                    wrapMode: Text.Wrap
                }

                DialogButton {
                    visible: root._minLevel > 0
                    Layout.alignment: Qt.AlignHCenter
                    Layout.topMargin: 6
                    implicitHeight: 28
                    text: "Show all levels"
                    onClicked: root._minLevel = 0
                }
            }

            // New entries while scrolled up: back to following
            AbstractButton {
                id: newEntriesPill
                visible: root._follow && !root._pinned && root._unseen > 0
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 12
                padding: 5
                leftPadding: 12
                rightPadding: 12
                focusPolicy: Qt.NoFocus
                onClicked: root._scrollToEnd()

                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }

                background: Rectangle {
                    radius: height / 2
                    color: newEntriesPill.hovered ? Qt.lighter(Theme.colorAccentGreen, 1.1) : Theme.colorAccentGreen
                }

                contentItem: RowLayout {
                    spacing: 6

                    Icon {
                        name: "chevron-down"
                        size: 12
                        color: Theme.colorTextOnAccent
                    }

                    Text {
                        text: root._number(root._unseen) + (root._unseen === 1 ? " new entry" : " new entries")
                        color: Theme.colorTextOnAccent
                        font.pixelSize: Theme.fontSizeSmall
                        font.weight: Font.Medium
                    }
                }
            }
        }

        // -- Log files
        Rectangle {
            visible: root.journal.filesAvailable
            Layout.fillWidth: true
            Layout.preferredHeight: filesRow.implicitHeight + 18
            color: "transparent"

            Divider {
                anchors.top: parent.top
                width: parent.width
            }

            RowLayout {
                id: filesRow
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                spacing: 8

                Icon {
                    name: "folder"
                    size: 13
                    color: Theme.colorTextSecondary
                }

                Text {
                    text: "Log files"
                    color: Theme.colorTextSecondary
                    font.pixelSize: Theme.fontSizeSmall
                }

                Text {
                    Layout.fillWidth: true
                    text: root.journal.displayFilePath
                    color: Theme.colorTextPrimary
                    font.family: Theme.fontFamilyMono
                    font.pixelSize: Theme.fontSizeSmall
                    elide: Text.ElideMiddle

                    HoverHandler {
                        id: pathHover
                    }

                    ToolTip.visible: pathHover.hovered
                    ToolTip.delay: 600
                    ToolTip.text: root.journal.logFilePath
                }

                DialogButton {
                    implicitHeight: 28
                    variant: "ghost"
                    iconName: "folder"
                    text: "Open folder"
                    onClicked: root.journal.openFolder()
                }
            }
        }

        // -- Log files couldn't be written: the journal is all there is
        Rectangle {
            visible: !root.journal.filesAvailable
            Layout.fillWidth: true
            Layout.preferredHeight: unavailableRow.implicitHeight + 18
            color: Theme.colorWarningSoft

            Rectangle {
                width: parent.width
                height: 1
                color: Theme.colorWarningLine
            }

            RowLayout {
                id: unavailableRow
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                spacing: 10

                Icon {
                    Layout.alignment: Qt.AlignTop
                    Layout.topMargin: 2
                    name: "circle-alert"
                    size: 14
                    color: Theme.colorWarning
                }

                Text {
                    Layout.fillWidth: true
                    textFormat: Text.StyledText
                    text: "<b>Log files couldn't be written</b> <font color=\"" + Theme.colorTextSecondary + "\">to " + root.journal.displayFolder + " (" + root.journal.filesError + "). This journal still records everything until Kemai quits — copy it before closing.</font>"
                    color: Theme.colorTextPrimary
                    font.pixelSize: Theme.fontSizeSmall
                    lineHeight: 1.3
                    wrapMode: Text.Wrap
                }
            }
        }

        Divider {}

        // -- Footer
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 16
            Layout.rightMargin: 16
            Layout.topMargin: 10
            Layout.bottomMargin: 10
            spacing: 8

            Text {
                Layout.fillWidth: true
                text: listed.count > 0 ? root._number(listed.count) + " listed" : "Nothing listed"
                color: Theme.colorTextSecondary
                font.pixelSize: Theme.fontSizeSmall
            }

            DialogButton {
                text: "Close"
                onClicked: root.close()
            }

            DialogButton {
                id: copyButton
                Layout.minimumWidth: 150
                variant: "primary"
                enabled: listed.count > 0
                iconName: root._copiedCount >= 0 ? "check" : "copy"
                text: root._copiedCount >= 0 ? "Copied " + root._entries(root._copiedCount) : "Copy listed entries"
                onClicked: root._copyListed()
                Keys.onReturnPressed: root._copyListed()
                Keys.onEnterPressed: root._copyListed()
            }
        }
    }
}
