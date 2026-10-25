// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kemai

// Number field with −/+ buttons, typeable, clamped to from..to; `edited` carries the new value
Rectangle {
    id: root

    property int value: 0
    property int from: 0
    property int to: 100

    signal edited(int value)

    function _set(v) {
        const clamped = Math.max(root.from, Math.min(root.to, v))
        if (clamped !== root.value)
            root.edited(clamped)
        valueInput.text = String(clamped)
    }

    implicitWidth: stepperRow.implicitWidth
    implicitHeight: 30
    radius: Theme.radiusMedium
    color: Theme.colorSurface
    opacity: root.enabled ? 1 : 0.4

    RowLayout {
        id: stepperRow
        anchors.fill: parent
        spacing: 0

        component StepButton: AbstractButton {
            id: stepButton
            required property string iconName
            Layout.preferredWidth: 28
            Layout.fillHeight: true
            focusPolicy: Qt.NoFocus
            autoRepeat: true

            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }

            contentItem: Item {
                Icon {
                    anchors.centerIn: parent
                    name: stepButton.iconName
                    size: 12
                    color: stepButton.hovered ? Theme.colorTextPrimary : Theme.colorTextSecondary
                }
            }
            background: Item {}
        }

        StepButton {
            iconName: "minus"
            onClicked: root._set(root.value - 1)
        }

        TextInput {
            id: valueInput
            Layout.preferredWidth: 28
            horizontalAlignment: TextInput.AlignHCenter
            text: String(root.value)
            color: Theme.colorTextPrimary
            selectionColor: Theme.colorAccentGreenDim
            font.pixelSize: Theme.fontSizeMedium
            font.weight: Font.DemiBold
            font.features: Theme.fontFeaturesTabular
            validator: IntValidator {
                bottom: 0
                top: 999
            }
            onEditingFinished: root._set(parseInt(valueInput.text) || root.from)
            Keys.onUpPressed: root._set(root.value + 1)
            Keys.onDownPressed: root._set(root.value - 1)

            Connections {
                target: root
                function onValueChanged() {
                    valueInput.text = String(root.value)
                }
            }
        }

        StepButton {
            iconName: "plus"
            onClicked: root._set(root.value + 1)
        }
    }
}
