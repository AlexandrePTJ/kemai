// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import Kemai

// "Required by Kimai" section of the time entry dialog: the required custom fields of the entry, in Kimai's order.
ColumnLayout {
    id: root

    required property CustomFieldsForm form
    property bool newEntry: false
    property url entryWebUrl
    // Project name shown on fields that appear after a project/activity change
    property string projectName: ""

    signal pickRequested(int row, bool dateOnly)

    // Moves the keyboard to the first missing or invalid field; returns its row item (to scroll to), or null
    function focusFirstIncomplete() {
        const row = root.form.firstIncompleteRow()
        const cell = row >= 0 ? fieldRepeater.itemAt(row) as FieldCell : null
        if (cell === null)
            return null
        cell.focusInput()
        return cell
    }

    spacing: 14

    ReferenceData {
        id: reference
    }

    // Read once: the lists are only needed when such a field exists
    readonly property var _countries: root._needs("country") ? reference.countries().map(c => ({
                key: c.code,
                label: c.name,
                code: c.code
            })) : []
    readonly property var _currencies: root._needs("currency") ? reference.currencies().map(c => ({
                key: c.code,
                label: c.name,
                code: c.code
            })) : []
    readonly property var _languages: root._needs("language") ? reference.languages().map(c => ({
                key: c.code,
                label: c.name,
                code: c.code
            })) : []

    function _needs(type) {
        return root.form.types.indexOf(type) >= 0
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: 1
        color: Theme.colorDivider
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: 8

        Text {
            text: "Required by Kimai"
            color: Theme.colorTextLabel
            font.pixelSize: Theme.fontSizeXSmall
            font.weight: Font.DemiBold
            font.letterSpacing: 1.3
            font.capitalization: Font.AllUppercase
        }

        Item {
            Layout.fillWidth: true
        }

        Text {
            visible: root.form.missingCount > 0
            text: root.form.missingCount + " to fill"
            color: Theme.colorWarning
            font.pixelSize: Theme.fontSizeXSmall
            font.weight: Font.DemiBold
        }

        RowLayout {
            visible: root.form.missingCount === 0
            spacing: 4

            Icon {
                name: "check"
                size: 11
                color: Theme.colorAccentGreen
            }

            Text {
                text: "All filled"
                color: Theme.colorTextSecondary
                font.pixelSize: Theme.fontSizeXSmall
            }
        }
    }

    Text {
        visible: root.newEntry
        Layout.fillWidth: true
        Layout.topMargin: -8
        text: "Kimai needs these values before the timer can start."
        color: Theme.colorTextSecondary
        font.pixelSize: Theme.fontSizeSmall
        wrapMode: Text.Wrap
    }

    component FieldCell: Rectangle {
        id: cell

        required property int index
        required property var model

        function focusInput() {
            fieldRow.focusInput()
        }

        Layout.fillWidth: true
        // A field that appeared: tinted, with an accent edge, both fading after 2 s
        Layout.leftMargin: cell.model.appeared ? -12 : 0
        Layout.rightMargin: cell.model.appeared ? -10 : 0
        Layout.preferredHeight: fieldRow.implicitHeight + (cell.model.appeared ? 20 : 0)
        radius: Theme.radiusMedium
        color: cell.model.appeared ? Theme.colorRowFresh : "transparent"

        Rectangle {
            visible: cell.model.appeared
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 2
            color: Theme.colorAccentGreen
        }

        Timer {
            running: cell.model.appeared
            interval: 2000
            onTriggered: fade.start()
        }

        ColorAnimation {
            id: fade
            target: cell
            property: "color"
            to: "transparent"
            duration: 400
        }

        CustomFieldRow {
            id: fieldRow
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: cell.model.appeared ? 12 : 0
            anchors.rightMargin: cell.model.appeared ? 10 : 0

            index: cell.index
            name: cell.model.name
            label: cell.model.label
            help: cell.model.help
            type: cell.model.type
            value: cell.model.value
            inputText: cell.model.inputText
            fieldState: cell.model.state
            error: cell.model.error
            choices: cell.model.choices
            appeared: cell.model.appeared
            unsaved: cell.model.unsaved

            form: root.form
            newEntry: root.newEntry
            entryWebUrl: root.entryWebUrl
            appearedFor: root.projectName
            countries: root._countries
            currencies: root._currencies
            languages: root._languages
            onPickRequested: (row, dateOnly) => root.pickRequested(row, dateOnly)
        }
    }

    Repeater {
        id: fieldRepeater
        model: root.form

        delegate: FieldCell {}
    }
}
