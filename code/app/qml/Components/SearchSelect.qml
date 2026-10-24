// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Select over a list of {key, label, meta?, color?, code?}: closed it shows the current item, open it
// filters as you type (label, code, meta; client side, fast for a few hundred items). An optional row
// under a divider offers to create something from the typed text.
FocusScope {
    id: root

    property var items: []
    property var currentKey: null
    property string placeholder: ""
    property bool hasError: false
    // Label of the create row ({query} is replaced by the typed text); empty: no create row
    property string createLabel: ""
    // Show the create row only when the typed text matches nothing
    property bool createOnlyWithoutMatch: false
    // Show the create row even without typed text (e.g. "New project…")
    property bool createAlways: false

    readonly property bool searching: dropdown.opened
    readonly property var currentItem: root._find(root.currentKey)

    signal selected(var item)
    signal createRequested(string query)

    function openSearch() {
        searchInput.text = ""
        dropdown.open()
        searchInput.forceActiveFocus()
    }

    function closeSearch() {
        dropdown.close()
        root.forceActiveFocus()
    }

    function _find(key) {
        for (let i = 0; i < root.items.length; ++i) {
            if (root.items[i].key === key)
                return root.items[i]
        }
        return null
    }

    readonly property var _filtered: {
        const q = searchInput.text.trim().toLowerCase()
        if (q === "")
            return root.items
        return root.items.filter(item => item.label.toLowerCase().includes(q)
                                 || (item.code !== undefined && item.code.toLowerCase().includes(q))
                                 || (item.meta !== undefined && item.meta.toLowerCase().includes(q)))
    }
    readonly property bool _showCreate: root.createLabel !== ""
                                        && (root.createAlways || (searchInput.text.trim() !== ""
                                                                  && (!root.createOnlyWithoutMatch || root._filtered.length === 0)))
    // Highlight over the filtered rows, then the create row
    property int _highlight: 0
    readonly property int _rowCount: root._filtered.length + (root._showCreate ? 1 : 0)

    function _highlightText(text, query) {
        const esc = (s) => s.replace(/&/g, "&amp;").replace(/</g, "&lt;")
        const i = query === "" ? -1 : text.toLowerCase().indexOf(query.toLowerCase())
        if (i < 0)
            return esc(text)
        return esc(text.slice(0, i)) + "<font color='" + Theme.colorAccentGreen + "'><b>"
               + esc(text.slice(i, i + query.length)) + "</b></font>" + esc(text.slice(i + query.length))
    }

    function _activate(index) {
        if (index < root._filtered.length) {
            root.selected(root._filtered[index])
            root.closeSearch()
        } else if (root._showCreate) {
            const query = searchInput.text.trim()
            root.closeSearch()
            root.createRequested(query)
        }
    }

    implicitHeight: 36
    activeFocusOnTab: true

    Keys.onPressed: (event) => {
        if (!root.searching && (event.key === Qt.Key_Return || event.key === Qt.Key_Enter
                                || event.key === Qt.Key_Space || event.key === Qt.Key_Down)) {
            root.openSearch()
            event.accepted = true
        }
    }

    Rectangle {
        anchors.fill: parent
        radius: Theme.radiusMedium
        color: Theme.colorSurface
        border.width: 1
        border.color: root.hasError ? Theme.colorAccentRed : root.searching || root.activeFocus ? Theme.colorAccentGreenDim : "transparent"

        Rectangle {
            visible: root.hasError || root.searching || root.activeFocus
            anchors.fill: parent
            anchors.margins: -3
            radius: Theme.radiusMedium + 3
            color: "transparent"
            border.width: 2
            border.color: root.hasError ? Theme.colorAccentRedSoft : Theme.colorAccentGreenSoft
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            enabled: !root.searching
            onClicked: root.openSearch()
        }

        // Closed: current item
        RowLayout {
            id: closedRow
            visible: !root.searching
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            spacing: 8

            Rectangle {
                visible: root.currentItem !== null && root.currentItem.color !== undefined
                Layout.preferredWidth: 10
                Layout.preferredHeight: 10
                radius: 2
                color: root.currentItem !== null && root.currentItem.color !== undefined ? root.currentItem.color : "transparent"
            }

            // The label gives way first: the meta (customer, code) stays readable
            Text {
                Layout.fillWidth: true
                Layout.maximumWidth: root.currentItem === null ? -1 : implicitWidth
                text: root.currentItem !== null ? root.currentItem.label : root.placeholder
                color: root.currentItem !== null ? Theme.colorTextPrimary : Theme.colorTextDim
                font.pixelSize: Theme.fontSizeMedium
                font.weight: root.currentItem !== null && root.currentItem.color !== undefined ? Font.Medium : Font.Normal
                elide: Text.ElideRight
            }

            // Meta, or the code (countries): "France FR"
            readonly property string _closedMeta: root.currentItem === null ? ""
                                                  : root.currentItem.meta !== undefined ? root.currentItem.meta
                                                  : root.currentItem.code !== undefined ? root.currentItem.code : ""

            Text {
                visible: closedRow._closedMeta !== ""
                Layout.maximumWidth: closedRow.width / 2
                text: closedRow._closedMeta
                color: Theme.colorTextSecondary
                font.pixelSize: Theme.fontSizeSmall
                elide: Text.ElideRight
            }

            Item {
                visible: root.currentItem !== null
                Layout.fillWidth: true
            }

            Icon {
                name: "chevron-down"
                size: 14
                color: Theme.colorTextSecondary
            }
        }

        // Open: search + count
        RowLayout {
            visible: root.searching
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            spacing: 8

            Icon {
                name: "search"
                size: 14
                color: Theme.colorTextSecondary
            }

            TextInput {
                id: searchInput
                Layout.fillWidth: true
                color: Theme.colorTextPrimary
                selectionColor: Theme.colorAccentGreenDim
                font.pixelSize: Theme.fontSizeMedium
                clip: true

                onTextChanged: root._highlight = 0

                Keys.onPressed: (event) => {
                    if (event.key === Qt.Key_Down) {
                        root._highlight = Math.min(root._highlight + 1, root._rowCount - 1)
                        listView.positionViewAtIndex(Math.min(root._highlight, root._filtered.length - 1), ListView.Contain)
                        event.accepted = true
                    } else if (event.key === Qt.Key_Up) {
                        root._highlight = Math.max(root._highlight - 1, 0)
                        listView.positionViewAtIndex(root._highlight, ListView.Contain)
                        event.accepted = true
                    } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                        root._activate(root._highlight)
                        event.accepted = true
                    } else if (event.key === Qt.Key_Escape) {
                        root.closeSearch()
                        event.accepted = true
                    }
                }
            }

            Text {
                text: root._filtered.length + " of " + root.items.length
                color: Theme.colorTextSecondary
                font.pixelSize: Theme.fontSizeXSmall
                font.features: Theme.fontFeaturesTabular
            }
        }
    }

    Popup {
        id: dropdown
        parent: root
        y: root.height + 4
        width: Math.max(root.width, 240)
        padding: 4
        // Pressing the field itself must not close and reopen it
        closePolicy: Popup.CloseOnPressOutsideParent

        onClosed: searchInput.text = ""

        background: Rectangle {
            color: Theme.colorBackground
            radius: Theme.radiusMedium
            border.width: 1
            border.color: Theme.colorBorderSubtle
        }

        contentItem: ColumnLayout {
            spacing: 0

            ListView {
                id: listView

                WheelScroller {
                    flickable: listView
                }
                Layout.fillWidth: true
                Layout.preferredHeight: Math.min(contentHeight, 7 * 32)
                visible: root._filtered.length > 0
                clip: true
                model: root._filtered
                boundsBehavior: Flickable.StopAtBounds

                delegate: Rectangle {
                    id: row

                    required property var modelData
                    required property int index

                    readonly property bool highlighted: root._highlight === row.index || rowHover.hovered

                    width: listView.width
                    height: 32
                    radius: Theme.radiusSmall
                    color: row.highlighted ? Theme.colorSurface : "transparent"

                    HoverHandler {
                        id: rowHover
                        cursorShape: Qt.PointingHandCursor
                    }

                    TapHandler {
                        onTapped: root._activate(row.index)
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10
                        spacing: 8

                        Rectangle {
                            visible: row.modelData.color !== undefined
                            Layout.preferredWidth: 8
                            Layout.preferredHeight: 8
                            radius: 2
                            color: row.modelData.color !== undefined ? row.modelData.color : "transparent"
                        }

                        Text {
                            visible: row.modelData.code !== undefined
                            Layout.preferredWidth: 32
                            text: row.modelData.code !== undefined ? row.modelData.code : ""
                            color: Theme.colorTextSecondary
                            font.pixelSize: Theme.fontSizeXSmall
                            font.features: Theme.fontFeaturesTabular
                        }

                        Text {
                            Layout.maximumWidth: implicitWidth
                            Layout.fillWidth: true
                            text: root._highlightText(row.modelData.label, searchInput.text.trim())
                            textFormat: Text.StyledText
                            color: Theme.colorTextPrimary
                            font.pixelSize: Theme.fontSizeMedium
                            elide: Text.ElideRight
                        }

                        Text {
                            visible: row.modelData.meta !== undefined
                            Layout.fillWidth: true
                            text: row.modelData.meta !== undefined ? row.modelData.meta : ""
                            color: Theme.colorTextSecondary
                            font.pixelSize: Theme.fontSizeSmall
                            elide: Text.ElideRight
                        }

                        Item {
                            visible: row.modelData.meta === undefined
                            Layout.fillWidth: true
                        }

                        Icon {
                            visible: row.modelData.key === root.currentKey
                            name: "check"
                            size: 13
                            color: Theme.colorAccentGreen
                        }
                    }
                }
            }

            Text {
                visible: root._filtered.length === 0
                Layout.fillWidth: true
                Layout.margins: 8
                text: "No match"
                color: Theme.colorTextSecondary
                font.pixelSize: Theme.fontSizeSmall
            }

            Rectangle {
                visible: root._showCreate
                Layout.fillWidth: true
                Layout.leftMargin: -4
                Layout.rightMargin: -4
                Layout.topMargin: 4
                Layout.bottomMargin: 4
                Layout.preferredHeight: 1
                color: Theme.colorDivider
            }

            Rectangle {
                id: createRow
                visible: root._showCreate
                readonly property bool highlighted: root._highlight === root._filtered.length || createHover.hovered
                Layout.fillWidth: true
                Layout.preferredHeight: 32
                radius: Theme.radiusSmall
                color: createRow.highlighted ? Theme.colorSurface : "transparent"

                HoverHandler {
                    id: createHover
                    cursorShape: Qt.PointingHandCursor
                }

                TapHandler {
                    onTapped: root._activate(root._filtered.length)
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    spacing: 8

                    Icon {
                        name: "plus"
                        size: 13
                        color: Theme.colorAccentGreen
                    }

                    Text {
                        Layout.fillWidth: true
                        textFormat: Text.StyledText
                        text: root.createLabel.replace("{query}", "<b>“" + searchInput.text.trim().replace(/&/g, "&amp;").replace(/</g, "&lt;") + "”</b>")
                        color: Theme.colorTextPrimary
                        font.pixelSize: Theme.fontSizeMedium
                        elide: Text.ElideRight
                    }
                }
            }
        }
    }
}
