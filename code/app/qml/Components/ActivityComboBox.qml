// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
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

    function _updateDropdown() {
        // Suppress the popup while the field still contains the committed
        // selection — the user already picked, no need to nag them. Editing
        // the text (even by one char) clears this branch and re-enables the
        // popup.
        if (_committedLabel !== "" && inputField.text === _committedLabel) {
            if (dropdown.opened) dropdown.close()
            return
        }
        if (inputField.activeFocus && suggestionList.count > 0) {
            if (!dropdown.opened) dropdown.open()
        } else if (suggestionList.count === 0 || !dropdown.hovered) {
            if (dropdown.opened) dropdown.close()
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

        contentItem: ListView {
            id: suggestionList
            clip: true
            model: filterProxy
            implicitHeight: Math.min(contentHeight, 240)
            onCountChanged: root._updateDropdown()

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
