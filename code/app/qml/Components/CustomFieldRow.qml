// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kemai

// One required custom field of the time entry dialog: label, help, an input fitted to Kimai's type, and the
// message line (amber while empty, red when the value does not fit). Values go to `form` (CustomFieldsForm).
ColumnLayout {
    id: root

    required property int index
    required property string name
    required property string label
    required property string help
    required property string type
    required property string value
    required property string inputText
    required property string fieldState
    required property string error
    required property var choices
    required property bool appeared
    required property bool unsaved

    required property CustomFieldsForm form
    // New entry mode: nothing exists in Kimai yet (no "Open in Kimai" link)
    property bool newEntry: false
    property url entryWebUrl
    // {key, label, code} lists of country, currency and language fields
    property var countries: []
    property var currencies: []
    property var languages: []
    // Name of the project the field appeared for, after a project/activity change
    property string appearedFor: ""

    // A date or date-time chip was clicked: the dialog opens its picker and calls form.setDate/setDateTime
    signal pickRequested(int row, bool dateOnly)

    // A red message waits for the first blur, then follows every change; amber shows at once
    property bool _errorShown: false
    readonly property bool _typed: ["text", "textarea", "integer", "number", "money", "duration", "email", "url"].indexOf(root.type) >= 0
    readonly property bool _inputFocused: textInput.activeFocus || longInput.activeFocus || hexInput.activeFocus
    readonly property bool _showInvalid: root.fieldState === "invalid" && (root._errorShown || !root._typed || !root._inputFocused)
    readonly property bool _showMissing: root.fieldState === "missing"
    readonly property bool _isList: root.type === "choice" || root.type === "choice-search" || root.type === "country" || root.type === "currency" || root.type === "language"

    // Puts the keyboard in the field (first missing field when the dialog opens)
    function focusInput() {
        if (root.type === "textarea")
            longInput.forceActiveFocus()
        else if (root._typed)
            textInput.forceActiveFocus()
        else if (root._isList)
            listInput.forceActiveFocus()
        else if (root.type === "color-free")
            hexInput.forceActiveFocus()
        else
            root.forceActiveFocus()
    }

    function _message() {
        if (root._showMissing)
            return root.type === "boolean" ? "Must be checked" : "Required"
        if (!root._showInvalid)
            return ""
        switch (root.error) {
        case "NotWholeNumber":
            return "Must be a whole number"
        case "NotNumber":
        case "NotMoney":
            return "Not a number"
        case "NotDuration":
            return "Not a duration — use h:mm"
        case "MinutesOutOfRange":
            return "Minutes go from 00 to 59"
        case "NotDate":
            return "Not a date"
        case "NotDateTime":
            return "Not a date and time"
        case "NotEmail":
            return "Not a valid email address"
        case "NotUrl":
            return "Not a valid URL — start with https://"
        case "NotColor":
            return "Not a color — use #rrggbb"
        case "NotInOptions":
            return "Not one of the options"
        case "UnknownCountry":
            return "Unknown country"
        case "UnknownCurrency":
            return "Unknown currency code"
        case "UnknownLanguage":
            return "Unknown language"
        default:
            return "Not a valid value"
        }
    }

    function _listItems() {
        switch (root.type) {
        case "country":
            return root.countries
        case "currency":
            return root.currencies
        case "language":
            return root.languages
        default:
            return root.choices
        }
    }

    function _border(focused) {
        if (root._showInvalid)
            return Theme.colorAccentRed
        if (root._showMissing)
            return Theme.colorWarningLine
        return focused ? Theme.colorAccentGreenDim : "transparent"
    }

    // "Fri 30 Oct 2026" / "Wed 28 Oct · 14:00"
    function _chipText() {
        if (root.fieldState === "missing" || root.value === "")
            return ""
        const d = root.form.dateTimeAt(root.index)
        if (isNaN(d.getTime()))
            return root.value
        if (root.type === "date")
            return d.toLocaleDateString(Qt.locale(), "ddd d MMM yyyy")
        return d.toLocaleDateString(Qt.locale(), "ddd d MMM") + " · " + d.toLocaleTimeString(Qt.locale(), "HH:mm")
    }

    on_InputFocusedChanged: {
        if (!root._inputFocused && root.fieldState === "invalid")
            root._errorShown = true
    }

    spacing: 0
    Layout.fillWidth: true

    // -- Label line (yes/no fields carry their label in the checkbox row)
    RowLayout {
        visible: root.type !== "boolean"
        Layout.fillWidth: true
        Layout.bottomMargin: root.help !== "" ? 2 : 6
        spacing: 8

        Text {
            Layout.fillWidth: !root.appeared && !root.unsaved
            text: root.label
            color: Theme.colorTextPrimary
            font.pixelSize: Theme.fontSizeMedium
            font.weight: Font.Medium
            wrapMode: Text.Wrap
        }

        Text {
            visible: root.appeared && root.appearedFor !== ""
            text: "for " + root.appearedFor
            color: Theme.colorAccentGreen
            font.pixelSize: Theme.fontSizeTiny
            font.weight: Font.DemiBold
            font.letterSpacing: 0.3
        }

        UnsavedTag {
            visible: root.unsaved
        }

        Item {
            visible: root.appeared || root.unsaved
            Layout.fillWidth: true
        }
    }

    Text {
        visible: root.help !== "" && root.type !== "boolean"
        Layout.fillWidth: true
        Layout.bottomMargin: 6
        text: root.help
        color: Theme.colorTextSecondary
        font.pixelSize: Theme.fontSizeSmall
        wrapMode: Text.Wrap
    }

    component UnsavedTag: Rectangle {
        Layout.preferredWidth: unsavedText.implicitWidth + 10
        Layout.preferredHeight: 16
        radius: 3
        color: Theme.colorWarningSoft
        border.width: 1
        border.color: Theme.colorWarningLine

        Text {
            id: unsavedText
            anchors.centerIn: parent
            text: "not saved"
            color: Theme.colorWarning
            font.pixelSize: Theme.fontSizeTiny
            font.weight: Font.DemiBold
        }
    }

    // Input box: 34 px, state border, focus ring
    component InputBox: Rectangle {
        id: box

        property bool focused: false

        Layout.preferredHeight: 34
        radius: Theme.radiusMedium
        color: Theme.colorSurface
        border.width: 1
        border.color: root._border(box.focused)

        Rectangle {
            visible: box.focused || root._showInvalid
            anchors.fill: parent
            anchors.margins: -3
            radius: Theme.radiusMedium + 3
            color: "transparent"
            border.width: 2
            border.color: root._showInvalid ? Theme.colorAccentRedSoft : Theme.colorAccentGreenSoft
        }
    }

    // -- Single-line typed values
    InputBox {
        visible: root._typed && root.type !== "textarea"
        focused: textInput.activeFocus
        Layout.fillWidth: !(root.type === "integer" || root.type === "number" || root.type === "duration")
        Layout.preferredWidth: 140

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            spacing: 8

            Icon {
                visible: root.type === "duration"
                name: "clock"
                size: 13
                color: Theme.colorTextSecondary
            }

            TextInput {
                id: textInput
                Layout.fillWidth: true
                clip: true
                color: Theme.colorTextPrimary
                selectionColor: Theme.colorAccentGreenDim
                selectByMouse: true
                font.pixelSize: Theme.fontSizeMedium
                font.features: Theme.fontFeaturesTabular
                inputMethodHints: root.type === "integer" ? Qt.ImhFormattedNumbersOnly : root.type === "number" || root.type === "money" ? Qt.ImhFormattedNumbersOnly : root.type === "email" ? Qt.ImhEmailCharactersOnly : root.type === "url" ? Qt.ImhUrlCharactersOnly : Qt.ImhNone
                activeFocusOnTab: true
                Component.onCompleted: textInput.text = root.inputText
                onTextEdited: root.form.setInput(root.index, textInput.text)

                Text {
                    visible: textInput.text === ""
                    anchors.verticalCenter: parent.verticalCenter
                    text: root.type === "duration" ? "0:00" : root.type === "money" ? Qt.locale().toString(0, "f", 2) : root.type === "integer" ? "0" : root.type === "number" ? Qt.locale().toString(0, "f", 1) : ""
                    color: Theme.colorTextDim
                    font.pixelSize: Theme.fontSizeMedium
                    font.features: Theme.fontFeaturesTabular
                }
            }

            Text {
                visible: root.type === "duration" || (root.type === "money" && root.form.currency !== "")
                text: root.type === "duration" ? "h:mm" : root.form.currency
                color: Theme.colorTextSecondary
                font.pixelSize: Theme.fontSizeXSmall
            }
        }
    }

    // -- Long text: grows up to 5 lines, then scrolls
    InputBox {
        visible: root.type === "textarea"
        focused: longInput.activeFocus
        Layout.fillWidth: true
        Layout.preferredHeight: Math.max(60, Math.min(longInput.implicitHeight, 5 * longInput.font.pixelSize * 1.4 + 16))

        Flickable {
            id: longFlick
            anchors.fill: parent
            clip: true
            contentHeight: longInput.implicitHeight
            boundsBehavior: Flickable.StopAtBounds

            TextEdit {
                id: longInput
                width: longFlick.width
                topPadding: 8
                bottomPadding: 8
                leftPadding: 10
                rightPadding: 10
                wrapMode: TextEdit.Wrap
                color: Theme.colorTextPrimary
                selectionColor: Theme.colorAccentGreenDim
                selectByMouse: true
                font.pixelSize: Theme.fontSizeMedium
                activeFocusOnTab: true
                // Tab moves on instead of inserting a tab character
                KeyNavigation.priority: KeyNavigation.BeforeItem
                Component.onCompleted: longInput.text = root.inputText
                onTextChanged: {
                    if (longInput.activeFocus)
                        root.form.setInput(root.index, longInput.text)
                }
                onCursorRectangleChanged: {
                    const r = longInput.cursorRectangle
                    if (r.y < longFlick.contentY)
                        longFlick.contentY = r.y
                    else if (r.y + r.height > longFlick.contentY + longFlick.height)
                        longFlick.contentY = r.y + r.height - longFlick.height + longInput.bottomPadding
                }
            }
        }
    }

    // -- Yes/no: checked is required
    AbstractButton {
        id: checkRow
        visible: root.type === "boolean"
        Layout.fillWidth: true
        focusPolicy: Qt.StrongFocus
        padding: 0
        onClicked: root.form.setValue(root.index, root.value === "1" ? "0" : "1")

        HoverHandler {
            cursorShape: Qt.PointingHandCursor
        }

        background: Rectangle {
            radius: Theme.radiusMedium
            color: root.value === "1" ? Theme.colorAccentGreenSoft : "transparent"
            border.width: 1
            border.color: root._showMissing ? Theme.colorWarningLine : checkRow.visualFocus ? Theme.colorAccentGreenDim : Theme.colorBorder
        }

        contentItem: RowLayout {
            spacing: 10

            Rectangle {
                Layout.alignment: Qt.AlignTop
                Layout.topMargin: 9
                Layout.leftMargin: 10
                Layout.preferredWidth: 16
                Layout.preferredHeight: 16
                radius: 3
                color: root.value === "1" ? Theme.colorAccentGreen : "transparent"
                border.width: root.value === "1" ? 0 : 1.5
                border.color: Theme.colorTextSecondary

                Icon {
                    visible: root.value === "1"
                    anchors.centerIn: parent
                    name: "check"
                    size: 11
                    color: Theme.colorTextOnAccent
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.topMargin: 8
                Layout.bottomMargin: 8
                Layout.rightMargin: 10
                spacing: 4

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text {
                        Layout.fillWidth: true
                        text: root.label
                        color: Theme.colorTextPrimary
                        font.pixelSize: Theme.fontSizeMedium
                        wrapMode: Text.Wrap
                    }

                    UnsavedTag {
                        visible: root.unsaved
                    }
                }

                Text {
                    visible: root.help !== ""
                    Layout.fillWidth: true
                    text: root.help
                    color: Theme.colorTextSecondary
                    font.pixelSize: Theme.fontSizeSmall
                    wrapMode: Text.Wrap
                }
            }
        }
    }

    // -- Date, date and time: a chip opening the picker
    AbstractButton {
        id: dateChip
        visible: root.type === "date" || root.type === "datetime"
        Layout.preferredWidth: root.type === "date" ? 200 : 240
        Layout.preferredHeight: 34
        focusPolicy: Qt.StrongFocus
        padding: 0
        onClicked: root.pickRequested(root.index, root.type === "date")
        Keys.onSpacePressed: root.pickRequested(root.index, root.type === "date")

        HoverHandler {
            cursorShape: Qt.PointingHandCursor
        }

        background: InputBox {
            focused: dateChip.visualFocus
        }

        contentItem: RowLayout {
            spacing: 8

            Icon {
                Layout.leftMargin: 10
                name: root.type === "date" ? "calendar" : "clock"
                size: 14
                color: Theme.colorTextSecondary
            }

            Text {
                Layout.fillWidth: true
                text: root._chipText() !== "" ? root._chipText() : root.type === "date" ? "Pick a date" : "Pick date and time"
                color: root._chipText() !== "" ? Theme.colorTextPrimary : Theme.colorTextDim
                font.pixelSize: Theme.fontSizeMedium
                font.features: Theme.fontFeaturesTabular
                elide: Text.ElideRight
            }

            Icon {
                Layout.rightMargin: 10
                name: "chevron-down"
                size: 13
                color: Theme.colorTextSecondary
            }
        }
    }

    // -- Choice, country, currency, language: searchable list
    SearchSelect {
        id: listInput
        visible: root._isList
        Layout.fillWidth: true
        Layout.preferredHeight: 34
        items: root._isList ? root._listItems() : []
        currentKey: root.value
        placeholder: "Choose…"
        hasError: root._showInvalid
        onSelected: item => root.form.setValue(root.index, item.key)
    }

    // A stored choice Kimai no longer offers: shown struck through
    Text {
        visible: root._isList && root.error === "NotInOptions" && root.value !== ""
        Layout.topMargin: 4
        text: root.value
        font.strikeout: true
        color: Theme.colorTextSecondary
        font.pixelSize: Theme.fontSizeSmall
    }

    // -- Colors: the instance's palette, plus a #hex field for a free color
    RowLayout {
        visible: root.type === "color" || root.type === "color-free"
        Layout.fillWidth: true
        spacing: 10

        Rectangle {
            visible: root.form.colorChoices.length > 0
            Layout.fillWidth: true
            Layout.preferredHeight: swatches.implicitHeight + 8
            radius: Theme.radiusMedium
            color: "transparent"
            border.width: 1
            border.color: root._showMissing ? Theme.colorWarningLine : root._showInvalid && root.type === "color" ? Theme.colorAccentRed : "transparent"

            Flow {
                id: swatches
                anchors.fill: parent
                anchors.margins: 4
                spacing: 4

                Repeater {
                    model: root.form.colorChoices

                    delegate: Rectangle {
                        id: swatch

                        required property var modelData
                        readonly property bool selected: root.value.toLowerCase() === swatch.modelData.color.toLowerCase()

                        width: 20
                        height: 20
                        radius: 4
                        color: swatch.modelData.color
                        border.width: 1
                        border.color: Qt.rgba(1, 1, 1, 0.12)

                        // Selected: a ring in the panel color, then one in the text color
                        Rectangle {
                            visible: swatch.selected
                            anchors.fill: parent
                            anchors.margins: -4
                            radius: 7
                            color: "transparent"
                            border.width: 2
                            border.color: Theme.colorTextPrimary
                        }

                        ToolTip.visible: swatchHover.hovered
                        ToolTip.text: swatch.modelData.name
                        ToolTip.delay: 500

                        HoverHandler {
                            id: swatchHover
                            cursorShape: Qt.PointingHandCursor
                        }

                        TapHandler {
                            gesturePolicy: TapHandler.ReleaseWithinBounds
                            onTapped: {
                                root.form.setValue(root.index, swatch.modelData.color)
                                hexInput.text = swatch.modelData.color
                            }
                        }
                    }
                }
            }
        }

        // A free color, or any color when the instance gave no palette
        InputBox {
            visible: root.type === "color-free" || root.form.colorChoices.length === 0
            focused: hexInput.activeFocus
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: 116

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                spacing: 8

                Rectangle {
                    Layout.preferredWidth: 12
                    Layout.preferredHeight: 12
                    radius: 3
                    color: root.fieldState === "ok" ? root.value : "transparent"
                    border.width: 1
                    border.color: Theme.colorBorderHighlight
                }

                TextInput {
                    id: hexInput
                    Layout.fillWidth: true
                    clip: true
                    maximumLength: 7
                    color: Theme.colorTextPrimary
                    selectionColor: Theme.colorAccentGreenDim
                    selectByMouse: true
                    font.pixelSize: Theme.fontSizeMedium
                    font.features: Theme.fontFeaturesTabular
                    activeFocusOnTab: true
                    Component.onCompleted: hexInput.text = root.value
                    onTextEdited: root.form.setInput(root.index, hexInput.text)

                    Text {
                        visible: hexInput.text === ""
                        anchors.verticalCenter: parent.verticalCenter
                        text: "#hex"
                        color: Theme.colorTextDim
                        font.pixelSize: Theme.fontSizeMedium
                    }
                }
            }
        }
    }

    // -- Kemai can't fill it: never blocks, never counts as missing
    Rectangle {
        visible: root.fieldState === "unsupported"
        Layout.fillWidth: true
        Layout.preferredHeight: Math.max(34, unsupportedRow.implicitHeight + 12)
        radius: Theme.radiusMedium
        color: "transparent"

        DashedBorder {}

        RowLayout {
            id: unsupportedRow
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            spacing: 8

            Icon {
                name: "lock"
                size: 13
                color: Theme.colorTextSecondary
            }

            Text {
                Layout.fillWidth: true
                text: root.newEntry ? "To fill in Kimai, once the timer has started" : "To fill in Kimai"
                color: Theme.colorTextSecondary
                font.pixelSize: Theme.fontSizeMedium
                wrapMode: Text.Wrap
            }

            AbstractButton {
                id: openLink
                visible: !root.newEntry
                focusPolicy: Qt.StrongFocus
                padding: 2
                onClicked: Qt.openUrlExternally(root.entryWebUrl)

                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }

                contentItem: RowLayout {
                    spacing: 5

                    Text {
                        text: "Open in Kimai"
                        color: Theme.colorAccentGreen
                        font.pixelSize: Theme.fontSizeSmall
                        font.weight: Font.Medium
                        font.underline: openLink.hovered || openLink.visualFocus
                    }

                    Icon {
                        name: "external-link"
                        size: 12
                        color: Theme.colorAccentGreen
                    }
                }

                background: Item {}
            }
        }
    }

    // -- Message line
    RowLayout {
        visible: root._message() !== ""
        Layout.topMargin: 6
        spacing: 6

        Icon {
            name: "circle-alert"
            size: 12
            color: root._showMissing ? Theme.colorWarning : Theme.colorAccentRed
        }

        Text {
            text: root._message()
            color: root._showMissing ? Theme.colorWarning : Theme.colorAccentRed
            font.pixelSize: Theme.fontSizeSmall
        }
    }
}
