// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Dialog footer button: "subtle", "primary" or "danger", with an optional leading icon.
// While busy it keeps full opacity and a wait cursor (the caller ignores clicks);
// `spinning` shows the spinner, which callers delay so fast requests don't flash it.
AbstractButton {
    id: root

    property string variant: "subtle"
    property string iconName: ""
    property bool busy: false
    property bool spinning: false

    readonly property bool _filled: root.variant !== "subtle"

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
                color: "white"
            }

            Icon {
                visible: !(root.busy && root.spinning) && root.iconName !== ""
                name: root.iconName
                size: 14
                color: root._filled ? "white" : Theme.colorTextPrimary
            }

            Text {
                text: root.text
                color: root._filled ? "white" : Theme.colorTextPrimary
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
            return root.hovered && root.enabled ? Theme.colorSurface : Theme.colorSurfaceDark
        }
        border.width: 1
        border.color: root.visualFocus ? Theme.colorAccentGreenDim : (root._filled ? "transparent" : Theme.colorBorderSubtle)

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
