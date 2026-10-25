// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import Kemai

Item {
    id: root

    required property ActivityListModel sourceModel
    property string placeholderText: ""
    property alias text: inputField.text
    readonly property int selectedActivityId: inputField.text === _committedLabel ? _committedActivityId : 0
    readonly property int selectedProjectId: inputField.text === _committedLabel ? _committedProjectId : 0

    property int _committedActivityId: 0
    property int _committedProjectId: 0
    property string _committedLabel: ""

    signal activitySelected(string label)
    // The search found nothing and the user chose to create it (kind: "activity" | "project")
    signal createRequested(string kind, string name)

    // Selects a pair given by the creation dialog (or any {projectId, activityId, label})
    function selectPair(pair) {
        root._committedActivityId = pair.activityId
        root._committedProjectId = pair.projectId
        root._committedLabel = pair.label
        inputField.text = pair.label
        dropdown.close()
        inputField.focus = false
        root.activitySelected(pair.label)
    }

    // Typed text that matches nothing: offer to create it
    readonly property bool _offering: inputField.activeFocus && inputField.text.trim() !== "" && inputField.text !== root._committedLabel && suggestionList.count === 0

    function _updateDropdown() {
        // Suppress the popup while the field still contains the committed
        // selection — the user already picked, no need to nag them. Editing
        // the text (even by one char) clears this branch and re-enables the
        // popup.
        if (_committedLabel !== "" && inputField.text === _committedLabel) {
            if (dropdown.opened)
                dropdown.close()
            return
        }
        if (inputField.activeFocus && (suggestionList.count > 0 || root._offering)) {
            if (!dropdown.opened)
                dropdown.open()
        } else if ((suggestionList.count === 0 && !root._offering) || !dropdownHover.hovered) {
            if (dropdown.opened)
                dropdown.close()
        }
    }

    implicitHeight: 40

    ActivityFilterProxyModel {
        id: filterProxy
        sourceModel: root.sourceModel
        filterText: inputField.text
    }

    TextField {
        id: inputField
        anchors.fill: parent
        placeholderText: root.placeholderText
        placeholderTextColor: Theme.colorTextSecondary
        color: Theme.colorTextPrimary
        font.pixelSize: Theme.fontSizeNormal
        leftPadding: 10
        rightPadding: 10

        background: Rectangle {
            color: Theme.colorSurface
            radius: Theme.radiusMedium
            implicitHeight: 40
        }

        onActiveFocusChanged: root._updateDropdown()
        onTextChanged: root._updateDropdown()

        Keys.onEscapePressed: {
            dropdown.close()
            focus = false
        }
        Keys.onUpPressed: if (root._offering)
            createOffer.moveHighlight(-1)
        Keys.onDownPressed: if (root._offering)
            createOffer.moveHighlight(1)
        Keys.onReturnPressed: if (root._offering)
            createOffer.activate()
        Keys.onEnterPressed: if (root._offering)
            createOffer.activate()
    }

    Popup {
        id: dropdown
        parent: root
        y: root.height + 2
        x: 0
        width: root.width
        padding: 4
        closePolicy: Popup.NoAutoClose

        background: Rectangle {
            color: Theme.colorSurface
            radius: Theme.radiusMedium
            border.color: Theme.colorBorderHighlight
            border.width: 1
        }

        contentItem: Item {
            implicitHeight: root._offering ? createOffer.implicitHeight : suggestionList.implicitHeight
            implicitWidth: dropdown.width - 8

            CreateOffer {
                id: createOffer
                visible: root._offering
                anchors.left: parent.left
                anchors.right: parent.right
                query: inputField.text
                onCreateRequested: (kind, name) => {
                    dropdown.close()
                    root.createRequested(kind, name)
                }
                onVisibleChanged: if (visible)
                    createOffer.highlight = 0
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
                implicitHeight: Math.min(contentHeight, 240)
                onCountChanged: root._updateDropdown()

                // Popup has no hovered property: track it to keep the list open while pointed at
                HoverHandler {
                    id: dropdownHover
                }

                delegate: ItemDelegate {
                    id: entryDelegate

                    required property int index
                    required property int activityId
                    required property int projectId
                    required property string label
                    required property string activityColor
                    required property string projectName

                    width: suggestionList.width
                    highlighted: suggestionList.currentIndex === entryDelegate.index

                    contentItem: Row {
                        spacing: 10
                        Rectangle {
                            width: 10
                            height: 10
                            radius: 5
                            color: entryDelegate.activityColor
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Text {
                            text: entryDelegate.label
                            color: Theme.colorTextPrimary
                            font.pixelSize: Theme.fontSizeNormal
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                        }
                    }

                    background: Rectangle {
                        color: entryDelegate.highlighted ? Theme.colorBackground : "transparent"
                        radius: Theme.radiusSmall
                    }

                    onClicked: {
                        root._committedActivityId = entryDelegate.activityId
                        root._committedProjectId = entryDelegate.projectId
                        root._committedLabel = entryDelegate.label
                        inputField.text = entryDelegate.label
                        root.activitySelected(entryDelegate.label)
                        dropdown.close()
                        inputField.focus = false
                    }
                }
            }
        }
    }
}
