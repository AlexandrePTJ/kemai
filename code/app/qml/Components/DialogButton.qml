// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kemai

// Dialog footer button: "subtle", "primary", "danger" or "ghost" (transparent, outlined; `tone: "warning"` turns
// its text and border amber), with an optional leading icon.
// While busy it keeps full opacity and a wait cursor (the caller ignores clicks);
// `spinning` shows the spinner, which callers delay so fast requests don't flash it.
AbstractButton {
    id: root

    property string variant: "subtle"
    property string tone: ""
    property string iconName: ""
    property bool busy: false
    property bool spinning: false

    readonly property bool _filled: root.variant === "primary" || root.variant === "danger"
    readonly property color _textColor: root._filled ? Theme.colorTextOnAccent : root.tone === "warning" ? Theme.colorWarning : Theme.colorTextPrimary

    implicitHeight: 34
    implicitWidth: Math.max(contentRow.implicitWidth + 28, 64)
    opacity: root.enabled || root.busy ? 1 : 0.45
    focusPolicy: Qt.StrongFocus

    HoverHandler {
        cursorShape: root.busy ? Qt.WaitCursor : (root.enabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor)
    }

    contentItem: Item {
        RowLayout {
            id: contentRow
            anchors.centerIn: parent
            spacing: 6

            Spinner {
                visible: root.busy && root.spinning
                color: Theme.colorTextOnAccent
            }

            Icon {
                visible: !(root.busy && root.spinning) && root.iconName !== ""
                name: root.iconName
                size: 14
                color: root._textColor
            }

            Text {
                text: root.text
                color: root._textColor
                font.pixelSize: Theme.fontSizeMedium
                font.weight: Font.Medium
            }
        }
    }

    background: Rectangle {
        radius: Theme.radiusMedium
        color: {
            if (root.variant === "primary")
                return root.hovered && root.enabled ? Qt.lighter(Theme.colorAccentGreen, 1.1) : Theme.colorAccentGreen
            if (root.variant === "danger")
                return root.hovered && root.enabled ? Qt.lighter(Theme.colorAccentRed, 1.1) : Theme.colorAccentRed
            if (root.variant === "ghost")
                return root.hovered && root.enabled ? Theme.colorSurfaceDark : "transparent"
            return root.hovered && root.enabled ? Theme.colorSurface : Theme.colorSurfaceDark
        }
        border.width: 1
        border.color: root.visualFocus ? Theme.colorAccentGreenDim : root._filled ? "transparent" : root.tone === "warning" ? Theme.colorWarningLine : Theme.colorBorderSubtle

        // Focus glow outside the border
        Rectangle {
            visible: root.visualFocus
            anchors.fill: parent
            anchors.margins: -3
            radius: Theme.radiusMedium + 3
            color: "transparent"
            border.width: 2
            border.color: Theme.colorAccentGreenSoft
        }
    }
}
