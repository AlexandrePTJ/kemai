// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kemai

// Save / Add button of the profile manager: primary when there is something to save, a spinner while saving
// (`spinning`, delayed by the caller), and a non-clickable "Saved" chip once saved
AbstractButton {
    id: root

    property bool busy: false
    property bool spinning: false
    property bool saved: false

    readonly property bool _active: root.enabled && !root.busy && !root.saved
    readonly property bool _filled: !root.saved && (root.enabled || root.busy)
    readonly property color _textColor: root.saved ? Theme.colorAccentGreen : root._filled ? Theme.colorTextOnAccent : Theme.colorTextDim

    implicitHeight: 34
    implicitWidth: Math.max(contentRow.implicitWidth + 28, 92)
    focusPolicy: Qt.StrongFocus

    HoverHandler {
        cursorShape: root.busy ? Qt.WaitCursor : (root._active ? Qt.PointingHandCursor : Qt.ArrowCursor)
    }

    contentItem: Item {
        RowLayout {
            id: contentRow
            anchors.centerIn: parent
            spacing: root.saved ? 6 : 8

            Spinner {
                visible: root.busy && root.spinning
                color: Theme.colorTextOnAccent
            }

            Icon {
                visible: root.saved
                name: "check"
                size: 14
                color: Theme.colorAccentGreen
            }

            Text {
                text: root.saved ? "Saved" : root.text
                color: root._textColor
                font.pixelSize: Theme.fontSizeMedium
                font.weight: Font.Medium
            }
        }
    }

    background: Rectangle {
        radius: Theme.radiusMedium
        color: {
            if (root.saved)
                return Theme.colorAccentGreenSoft
            if (!root._filled)
                return Theme.colorSurfaceDark
            return root.hovered && root._active ? Qt.lighter(Theme.colorAccentGreen, 1.1) : Theme.colorAccentGreen
        }
        border.width: 1
        border.color: root.saved ? Theme.colorAccentGreenDim : root.visualFocus ? Theme.colorAccentGreenDim : "transparent"

        // Focus glow outside the border
        Rectangle {
            visible: root.visualFocus && !root.saved
            anchors.fill: parent
            anchors.margins: -3
            radius: Theme.radiusMedium + 3
            color: "transparent"
            border.width: 2
            border.color: Theme.colorAccentGreenSoft
        }
    }
}
