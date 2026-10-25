// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Layouts

import Kemai

// Labelled form control: caps label (+ optional note), the control, and an error line under it.
// The error may carry a link (e.g. "Use it"): `linkText` and `linkActivated`.
ColumnLayout {
    id: root

    property string label: ""
    property string note: ""
    property string error: ""
    property string linkText: ""

    default property alias content: controlColumn.data

    signal linkActivated()

    spacing: 6

    RowLayout {
        spacing: 8

        Text {
            text: root.label.toUpperCase()
            color: Theme.colorTextLabel
            font.pixelSize: Theme.fontSizeXSmall
            font.weight: Font.DemiBold
            font.letterSpacing: 1.3
        }

        Text {
            visible: root.note !== ""
            text: root.note
            color: Theme.colorTextDim
            font.pixelSize: 10
        }
    }

    ColumnLayout {
        id: controlColumn
        Layout.fillWidth: true
        spacing: 0
    }

    RowLayout {
        visible: root.error !== ""
        Layout.fillWidth: true
        spacing: 6

        Icon {
            Layout.alignment: Qt.AlignTop
            Layout.topMargin: 1
            name: "circle-alert"
            size: 13
            color: Theme.colorAccentRed
        }

        Text {
            Layout.fillWidth: true
            textFormat: Text.StyledText
            text: root.error + (root.linkText !== ""
                                ? " <a href=\"link\"><font color=\"" + Theme.colorAccentGreen + "\"><b>" + root.linkText + "</b></font></a>" : "")
            color: Theme.colorAccentRed
            font.pixelSize: Theme.fontSizeSmall
            wrapMode: Text.Wrap
            onLinkActivated: root.linkActivated()

            HoverHandler {
                cursorShape: parent.hoveredLink !== "" ? Qt.PointingHandCursor : Qt.ArrowCursor
            }
        }
    }
}
