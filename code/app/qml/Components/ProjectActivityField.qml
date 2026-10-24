// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kemai

// Shows the current project — activity pair; opens into a search over the cached pairs.
// Only listed pairs can be picked, so the result is always valid.
FocusScope {
    id: root

    required property ActivityListModel sourceModel
    property int projectId: 0
    property int activityId: 0
    property string projectName: ""
    property string activityName: ""
    property string activityColor: "transparent"

    readonly property bool searching: dropdown.opened

    signal pairSelected(int projectId, int activityId, string projectName, string activityName, string activityColor)
    // The search found nothing and the user chose to create it (kind: "activity" | "project")
    signal createRequested(string kind, string name)

    readonly property bool _offering: root.searching && searchInput.text.trim() !== "" && suggestionList.count === 0

    function openSearch() {
        searchInput.text = ""
        dropdown.open()
        searchInput.forceActiveFocus()
    }

    function closeSearch() {
        dropdown.close()
        root.forceActiveFocus()
    }

    // Query part of `text` in accent, when it appears as is (fuzzy matches stay plain)
    function _highlight(text, query) {
        const esc = (s) => s.replace(/&/g, "&amp;").replace(/</g, "&lt;")
        const i = query === "" ? -1 : text.toLowerCase().indexOf(query.toLowerCase())
        if (i < 0)
            return esc(text)
        return esc(text.slice(0, i)) + "<font color='" + Theme.colorAccentGreen + "'><b>"
               + esc(text.slice(i, i + query.length)) + "</b></font>" + esc(text.slice(i + query.length))
    }

    function _pick(row) {
        root.pairSelected(row.projectId, row.activityId, row.projectName, row.activityName, row.activityColor)
        root.closeSearch()
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

    ActivityFilterProxyModel {
        id: filterProxy
        sourceModel: root.sourceModel
        filterText: searchInput.text
    }

    Rectangle {
        id: box
        anchors.fill: parent
        radius: Theme.radiusMedium
        color: Theme.colorSurface
        border.width: 1
        border.color: root.searching || root.activeFocus ? Theme.colorAccentGreenDim : "transparent"

        Rectangle {
            visible: root.searching || root.activeFocus
            anchors.fill: parent
            anchors.margins: -3
            radius: Theme.radiusMedium + 3
            color: "transparent"
            border.width: 2
            border.color: Theme.colorAccentGreenSoft
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            enabled: !root.searching
            onClicked: root.openSearch()
        }

        // Closed: current pair
        RowLayout {
            visible: !root.searching
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            spacing: 8

            Rectangle {
                Layout.preferredWidth: 10
                Layout.preferredHeight: 10
                radius: 3
                color: root.activityColor
            }

            Text {
                text: root.projectName
                color: Theme.colorTextPrimary
                font.pixelSize: Theme.fontSizeMedium
                font.weight: Font.DemiBold
            }

            Text {
                text: "—"
                color: Theme.colorTextDim
                font.pixelSize: Theme.fontSizeMedium
            }

            Text {
                Layout.fillWidth: true
                text: root.activityName
                color: Theme.colorTextSecondary
                font.pixelSize: Theme.fontSizeMedium
                elide: Text.ElideRight
            }

            Icon {
                name: "chevron-down"
                size: 14
            }
        }

        // Open: search input + match count
        RowLayout {
            visible: root.searching
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            spacing: 8

            Icon {
                name: "search"
                size: 14
            }

            TextInput {
                id: searchInput
                Layout.fillWidth: true
                color: Theme.colorTextPrimary
                selectionColor: Theme.colorAccentGreenDim
                font.pixelSize: Theme.fontSizeMedium
                clip: true

                onTextChanged: suggestionList.currentIndex = 0

                Keys.onPressed: (event) => {
                    if (event.key === Qt.Key_Down) {
                        if (root._offering)
                            createOffer.moveHighlight(1)
                        else
                            suggestionList.incrementCurrentIndex()
                        event.accepted = true
                    } else if (event.key === Qt.Key_Up) {
                        if (root._offering)
                            createOffer.moveHighlight(-1)
                        else
                            suggestionList.decrementCurrentIndex()
                        event.accepted = true
                    } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                        if (root._offering)
                            createOffer.activate()
                        else if (suggestionList.currentItem)
                            root._pick(suggestionList.currentItem)
                        event.accepted = true
                    } else if (event.key === Qt.Key_Escape) {
                        root.closeSearch()
                        event.accepted = true
                    }
                }
            }

            Text {
                text: suggestionList.count + " of " + root.sourceModel.count
                color: Theme.colorTextSecondary
                font.pixelSize: Theme.fontSizeXSmall
            }
        }
    }

    Popup {
        id: dropdown
        parent: root
        y: root.height + 4
        width: root.width
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

        contentItem: Item {
            implicitHeight: root._offering ? createOffer.implicitHeight : suggestionList.implicitHeight
            implicitWidth: dropdown.width - 8

        CreateOffer {
            id: createOffer
            visible: root._offering
            anchors.left: parent.left
            anchors.right: parent.right
            query: searchInput.text
            onCreateRequested: (kind, name) => {
                root.closeSearch()
                root.createRequested(kind, name)
            }
            onVisibleChanged: if (visible) createOffer.highlight = 0
        }

        ListView {
            id: suggestionList
            visible: !root._offering
            anchors.fill: parent

            WheelScroller {
                flickable: suggestionList
            }
            clip: true
            model: filterProxy
            implicitHeight: Math.min(contentHeight, 7 * 32)
            highlightMoveDuration: 0
            boundsBehavior: Flickable.StopAtBounds

            delegate: Rectangle {
                id: row

                required property int index
                required property int activityId
                required property int projectId
                required property string projectName
                required property string activityName
                required property string activityColor
                required property bool isGlobal

                readonly property bool highlighted: suggestionList.currentIndex === row.index || rowHover.hovered
                readonly property bool current: row.projectId === root.projectId && row.activityId === root.activityId

                width: suggestionList.width
                height: 32
                radius: Theme.radiusSmall
                color: row.highlighted ? Theme.colorSurface : "transparent"

                HoverHandler {
                    id: rowHover
                    cursorShape: Qt.PointingHandCursor
                }

                TapHandler {
                    onTapped: root._pick(row)
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    spacing: 8

                    Rectangle {
                        Layout.preferredWidth: 8
                        Layout.preferredHeight: 8
                        radius: 2
                        color: row.activityColor
                    }

                    Text {
                        text: root._highlight(row.projectName, searchInput.text)
                        textFormat: Text.StyledText
                        color: Theme.colorTextPrimary
                        font.pixelSize: Theme.fontSizeMedium
                        font.weight: Font.Medium
                    }

                    Text {
                        text: "—"
                        color: Theme.colorTextDim
                        font.pixelSize: Theme.fontSizeMedium
                    }

                    Text {
                        Layout.fillWidth: true
                        text: root._highlight(row.activityName, searchInput.text)
                        textFormat: Text.StyledText
                        color: Theme.colorTextSecondary
                        font.pixelSize: Theme.fontSizeMedium
                        elide: Text.ElideRight
                    }

                    Rectangle {
                        visible: row.isGlobal
                        Layout.preferredWidth: globalText.implicitWidth + 10
                        Layout.preferredHeight: globalText.implicitHeight + 2
                        radius: 3
                        color: "transparent"
                        border.width: 1
                        border.color: Theme.colorBorderSubtle

                        Text {
                            id: globalText
                            anchors.centerIn: parent
                            text: "global"
                            color: Theme.colorTextSecondary
                            font.pixelSize: 10
                        }
                    }

                    Icon {
                        visible: row.current
                        name: "check"
                        size: 13
                        color: Theme.colorAccentGreen
                    }
                }
            }
        }
        }
    }
}
