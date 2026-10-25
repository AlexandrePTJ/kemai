// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kemai

// One entry of the account and login menus: icon, label, optional sub-line (with a status dot) and trailing icon.
// Enter and Return activate it.
AbstractButton {
    id: root

    property string iconName
    property string detail: ""
    property color detailColor: Theme.colorTextSecondary
    property bool detailDot: false
    property string trailingIcon: ""

    Layout.fillWidth: true
    padding: 10
    topPadding: 8
    bottomPadding: 8
    focusPolicy: Qt.StrongFocus

    Keys.onReturnPressed: root.clicked()
    Keys.onEnterPressed: root.clicked()

    HoverHandler {
        cursorShape: Qt.PointingHandCursor
    }

    background: Rectangle {
        radius: Theme.radiusSmall
        color: root.hovered || root.activeFocus ? Theme.colorSurface : "transparent"
    }

    contentItem: RowLayout {
        spacing: 10

        Icon {
            Layout.alignment: root.detail !== "" ? Qt.AlignTop : Qt.AlignVCenter
            Layout.topMargin: root.detail !== "" ? 1 : 0
            name: root.iconName
            size: 15
            color: root.hovered || root.activeFocus ? Theme.colorTextPrimary : Theme.colorTextSecondary
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2

            Text {
                Layout.fillWidth: true
                text: root.text
                color: Theme.colorTextPrimary
                font.pixelSize: Theme.fontSizeMedium
                elide: Text.ElideRight
            }

            RowLayout {
                visible: root.detail !== ""
                spacing: 5

                Rectangle {
                    visible: root.detailDot
                    Layout.preferredWidth: 6
                    Layout.preferredHeight: 6
                    radius: 3
                    color: root.detailColor
                }

                Text {
                    text: root.detail
                    color: root.detailColor
                    font.pixelSize: Theme.fontSizeXSmall
                    font.features: Theme.fontFeaturesTabular
                }
            }
        }

        Icon {
            visible: root.trailingIcon !== ""
            name: root.trailingIcon
            size: 12
            color: Theme.colorTextDim
        }
    }
}
