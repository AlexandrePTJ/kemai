// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kemai

// Entry tags as removable chips, plus an input suggesting tags that exist on the server.
// A name the server does not know is added as "new": Kimai creates it when the entry is saved.
FocusScope {
    id: root

    required property SessionContext sessionContext
    property var tags: []
    // Tags added here that the server did not suggest
    property var newTags: []

    // idle | loading | results | empty | comma
    readonly property string _state: {
        const text = tagInput.text
        if (text.indexOf(",") >= 0)
            return "comma"
        if (text.trim() === "")
            return "idle"
        if (root._resultsQuery !== text.trim())
            return "loading"
        return root._results.length > 0 ? "results" : "empty"
    }
    property var _results: []
    property string _resultsQuery: ""
    property int _highlighted: 0

    function _has(name) {
        const lower = name.toLowerCase()
        return root.tags.some((t) => t.toLowerCase() === lower)
    }

    function _add(name, isNew) {
        const trimmed = name.trim()
        if (trimmed === "" || trimmed.indexOf(",") >= 0 || root._has(trimmed))
            return
        root.tags = root.tags.concat([trimmed])
        if (isNew)
            root.newTags = root.newTags.concat([trimmed])
        tagInput.text = ""
    }

    function _remove(name) {
        root.tags = root.tags.filter((t) => t !== name)
        root.newTags = root.newTags.filter((t) => t !== name)
    }

    function _selectable(index) {
        return index >= 0 && index < root._results.length && !root._has(root._results[index])
    }

    function _firstSelectable() {
        for (let i = 0; i < root._results.length; ++i) {
            if (root._selectable(i))
                return i
        }
        return -1
    }

    function _move(step) {
        for (let i = root._highlighted + step; i >= 0 && i < root._results.length; i += step) {
            if (root._selectable(i)) {
                root._highlighted = i
                return
            }
        }
    }

    // Enter: highlighted suggestion, else the typed name as a new tag
    function _validate() {
        if (root._state === "results" && root._selectable(root._highlighted))
            root._add(root._results[root._highlighted], false)
        else if (root._state === "empty")
            root._add(tagInput.text, true)
    }

    implicitHeight: chipsBox.implicitHeight

    Connections {
        target: root.sessionContext

        function onTagSuggestionsReady(query: string, tags: list<string>) {
            // Drop replies to a query the user has typed past
            if (query !== tagInput.text.trim())
                return
            root._results = tags
            root._resultsQuery = query
            root._highlighted = root._firstSelectable()
        }
    }

    Timer {
        id: debounce
        interval: 200
        onTriggered: {
            const query = tagInput.text.trim()
            if (query !== "" && query.indexOf(",") < 0)
                root.sessionContext.requestTagSuggestions(query)
        }
    }

    Rectangle {
        id: chipsBox
        anchors.left: parent.left
        anchors.right: parent.right
        implicitHeight: Math.max(36, chipsFlow.implicitHeight + 12)
        radius: Theme.radiusMedium
        color: Theme.colorSurface
        border.width: 1
        border.color: root._state === "comma" ? Theme.colorAccentRed
                      : (tagInput.activeFocus ? Theme.colorAccentGreenDim : "transparent")

        Rectangle {
            visible: tagInput.activeFocus
            anchors.fill: parent
            anchors.margins: -3
            radius: Theme.radiusMedium + 3
            color: "transparent"
            border.width: 2
            border.color: root._state === "comma" ? Theme.colorAccentRedSoft : Theme.colorAccentGreenSoft
        }

        TapHandler {

            gesturePolicy: TapHandler.ReleaseWithinBounds
            onTapped: tagInput.forceActiveFocus()
        }

        Flow {
            id: chipsFlow
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.margins: 6
            spacing: 6

            Repeater {
                model: root.tags

                delegate: Rectangle {
                    id: chip

                    required property string modelData
                    readonly property bool isNew: root.newTags.indexOf(chip.modelData) >= 0

                    width: chipRow.implicitWidth + 16
                    height: 24
                    radius: Theme.radiusSmall
                    color: Theme.colorAccentGreenSoft
                    border.width: chip.isNew ? 0 : 1
                    border.color: Theme.colorAccentGreenDim

                    DashedBorder {
                        visible: chip.isNew
                        color: Theme.colorAccentGreenDim
                        radius: Theme.radiusSmall
                    }

                    RowLayout {
                        id: chipRow
                        anchors.centerIn: parent
                        spacing: 6

                        Text {
                            text: chip.modelData
                            color: Theme.colorAccentGreen
                            font.pixelSize: Theme.fontSizeSmall
                        }

                        Text {
                            visible: chip.isNew
                            text: "new"
                            color: Theme.colorTextSecondary
                            font.pixelSize: Theme.fontSizeTiny
                        }

                        Icon {
                            name: "x"
                            size: 10
                            color: Theme.colorAccentGreen

                            TapHandler {

                                gesturePolicy: TapHandler.ReleaseWithinBounds
                                cursorShape: Qt.PointingHandCursor
                                onTapped: root._remove(chip.modelData)
                            }
                        }
                    }
                }
            }

            TextInput {
                id: tagInput
                width: Math.max(80, implicitWidth + 8)
                height: 24
                verticalAlignment: TextInput.AlignVCenter
                leftPadding: 4
                color: Theme.colorTextPrimary
                selectionColor: Theme.colorAccentGreenDim
                font.pixelSize: Theme.fontSizeMedium
                activeFocusOnTab: true
                focus: true

                Text {
                    visible: tagInput.text === "" && !tagInput.activeFocus
                    anchors.verticalCenter: parent.verticalCenter
                    leftPadding: 4
                    text: "Add tag…"
                    color: Theme.colorTextDim
                    font.pixelSize: Theme.fontSizeMedium
                }

                onTextChanged: debounce.restart()
                onActiveFocusChanged: activeFocus ? suggestions.open() : suggestions.close()

                Keys.onPressed: (event) => {
                    if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                        root._validate()
                        event.accepted = true
                    } else if (event.key === Qt.Key_Down) {
                        root._move(1)
                        event.accepted = true
                    } else if (event.key === Qt.Key_Up) {
                        root._move(-1)
                        event.accepted = true
                    } else if (event.key === Qt.Key_Backspace && tagInput.text === "" && root.tags.length > 0) {
                        root._remove(root.tags[root.tags.length - 1])
                        event.accepted = true
                    } else if (event.key === Qt.Key_Escape && suggestions.opened) {
                        suggestions.close()
                        event.accepted = true
                    }
                }
            }
        }
    }

    Popup {
        id: suggestions
        parent: chipsBox
        // Below the field, or above it when the window has no room left underneath
        y: {
            const spaceBelow = Overlay.overlay ? Overlay.overlay.height - chipsBox.mapToItem(null, 0, chipsBox.height).y : 1e6
            return spaceBelow >= suggestions.implicitHeight + 8 ? chipsBox.height + 4 : -suggestions.implicitHeight - 4
        }
        width: chipsBox.width
        padding: 4
        closePolicy: Popup.CloseOnPressOutsideParent

        background: Rectangle {
            color: Theme.colorBackground
            radius: Theme.radiusMedium
            border.width: 1
            border.color: Theme.colorBorderSubtle
        }

        contentItem: ColumnLayout {
            spacing: 0

            // Messages: idle, loading, comma
            RowLayout {
                visible: root._state === "idle" || root._state === "loading" || root._state === "comma"
                Layout.fillWidth: true
                Layout.margins: 7
                Layout.leftMargin: 10
                spacing: 8

                Spinner {
                    visible: root._state === "loading"
                    color: Theme.colorAccentGreen
                    trackColor: Theme.colorBorderSubtle
                }

                Icon {
                    visible: root._state === "comma"
                    name: "circle-alert"
                    size: 13
                    color: Theme.colorAccentRed
                }

                Text {
                    Layout.fillWidth: true
                    text: root._state === "loading" ? "Searching tags…"
                          : root._state === "comma" ? "Tags can't contain commas — add them one at a time."
                          : "Type to search tags from the server"
                    color: root._state === "comma" ? Theme.colorAccentRed : Theme.colorTextSecondary
                    font.pixelSize: Theme.fontSizeSmall
                    wrapMode: Text.Wrap
                }
            }

            // No match: offer the typed name as a new tag
            Text {
                visible: root._state === "empty"
                Layout.leftMargin: 10
                Layout.topMargin: 6
                Layout.bottomMargin: 4
                text: "No existing tag matches"
                color: Theme.colorTextSecondary
                font.pixelSize: Theme.fontSizeXSmall
            }

            Rectangle {
                visible: root._state === "empty"
                Layout.fillWidth: true
                Layout.preferredHeight: 32
                radius: Theme.radiusSmall
                color: Theme.colorSurface

                TapHandler {

                    gesturePolicy: TapHandler.ReleaseWithinBounds
                    cursorShape: Qt.PointingHandCursor
                    onTapped: root._add(tagInput.text, true)
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
                        text: "Add <b>“" + tagInput.text.trim().replace(/&/g, "&amp;").replace(/</g, "&lt;") + "”</b>"
                        textFormat: Text.StyledText
                        color: Theme.colorTextPrimary
                        font.pixelSize: Theme.fontSizeMedium
                        elide: Text.ElideRight
                    }

                    Text {
                        text: "created on save"
                        color: Theme.colorTextSecondary
                        font.pixelSize: Theme.fontSizeXSmall
                    }
                }
            }

            // Matches (a short query can match hundreds of tags)
            ListView {
                id: resultsList

                WheelScroller {
                    flickable: resultsList
                }
                visible: root._state === "results"
                Layout.fillWidth: true
                Layout.preferredHeight: Math.min(resultsList.contentHeight, 6 * 32)
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                model: root._state === "results" ? root._results : []
                currentIndex: root._highlighted
                highlightMoveDuration: 0

                delegate: Rectangle {
                    id: suggestion

                    required property string modelData
                    required property int index
                    readonly property bool added: root._has(suggestion.modelData)

                    width: resultsList.width
                    height: 32
                    radius: Theme.radiusSmall
                    color: !suggestion.added && (root._highlighted === suggestion.index || suggestionHover.hovered)
                           ? Theme.colorSurface : "transparent"

                    HoverHandler {
                        id: suggestionHover
                        enabled: !suggestion.added
                        cursorShape: Qt.PointingHandCursor
                    }

                    TapHandler {

                        gesturePolicy: TapHandler.ReleaseWithinBounds
                        enabled: !suggestion.added
                        onTapped: root._add(suggestion.modelData, false)
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10
                        spacing: 8

                        Icon {
                            name: "tag"
                            size: 12
                            color: suggestion.added ? Theme.colorTextDim : Theme.colorTextSecondary
                        }

                        Text {
                            Layout.fillWidth: true
                            text: {
                                const name = suggestion.modelData
                                const query = tagInput.text.trim()
                                const esc = (s) => s.replace(/&/g, "&amp;").replace(/</g, "&lt;")
                                const i = name.toLowerCase().indexOf(query.toLowerCase())
                                if (suggestion.added || query === "" || i < 0)
                                    return esc(name)
                                return esc(name.slice(0, i)) + "<font color='" + Theme.colorAccentGreen + "'><b>"
                                       + esc(name.slice(i, i + query.length)) + "</b></font>" + esc(name.slice(i + query.length))
                            }
                            textFormat: Text.StyledText
                            color: suggestion.added ? Theme.colorTextDim : Theme.colorTextPrimary
                            font.pixelSize: Theme.fontSizeMedium
                            elide: Text.ElideRight
                        }

                        Icon {
                            visible: suggestion.added
                            name: "check"
                            size: 11
                            color: Theme.colorTextDim
                        }

                        Text {
                            visible: suggestion.added
                            text: "added"
                            color: Theme.colorTextDim
                            font.pixelSize: Theme.fontSizeXSmall
                        }
                    }
                }
            }
        }
    }
}
