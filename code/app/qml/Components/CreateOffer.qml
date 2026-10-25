// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import Kemai

// Shown by the project — activity pickers when the search finds nothing: offers to create an activity
// (highlighted first, the common case) or a project named after the typed text. The picker forwards
// Up/Down/Enter through moveHighlight() and activate().
ColumnLayout {
    id: root

    property string query: ""
    property int highlight: 0

    // kind: "activity" | "project"; name: the trimmed query
    signal createRequested(string kind, string name)

    function moveHighlight(step) {
        root.highlight = Math.max(0, Math.min(1, root.highlight + step))
    }

    function activate() {
        root.createRequested(root.highlight === 0 ? "activity" : "project", root.query.trim())
    }

    function _escaped(text) {
        return text.replace(/&/g, "&amp;").replace(/</g, "&lt;")
    }

    spacing: 0

    Text {
        Layout.fillWidth: true
        Layout.leftMargin: 10
        Layout.rightMargin: 10
        Layout.topMargin: 7
        Layout.bottomMargin: 6
        text: "No project — activity matches “" + root.query.trim() + "”"
        color: Theme.colorTextSecondary
        font.pixelSize: Theme.fontSizeSmall
        elide: Text.ElideRight
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.leftMargin: -4
        Layout.rightMargin: -4
        Layout.bottomMargin: 4
        Layout.preferredHeight: 1
        color: Theme.colorDivider
    }

    Repeater {
        model: [
            { kind: "activity", title: "New activity", hint: "Inside a project you choose, or global" },
            { kind: "project", title: "New project", hint: "Under a customer, with an activity to track" }
        ]

        delegate: Rectangle {
            id: offer

            required property var modelData
            required property int index

            readonly property bool highlighted: root.highlight === offer.index

            Layout.fillWidth: true
            Layout.preferredHeight: offerRow.implicitHeight + 16
            radius: Theme.radiusSmall
            color: offer.highlighted ? Theme.colorSurface : "transparent"

            HoverHandler {
                cursorShape: Qt.PointingHandCursor
                onHoveredChanged: if (hovered) root.highlight = offer.index
            }

            TapHandler {
                onTapped: {
                    root.highlight = offer.index
                    root.activate()
                }
            }

            RowLayout {
                id: offerRow
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                spacing: 10

                Rectangle {
                    Layout.preferredWidth: 26
                    Layout.preferredHeight: 26
                    radius: Theme.radiusMedium
                    color: offer.highlighted ? Theme.colorAccentGreenSoft : Theme.colorSurfaceDark
                    border.width: 1
                    border.color: offer.highlighted ? Theme.colorAccentGreenDim : Theme.colorBorderSubtle

                    Icon {
                        anchors.centerIn: parent
                        name: "plus"
                        size: 13
                        color: offer.highlighted ? Theme.colorAccentGreen : Theme.colorTextSecondary
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 1

                    Text {
                        Layout.fillWidth: true
                        textFormat: Text.StyledText
                        text: offer.modelData.title + " <b>“" + root._escaped(root.query.trim()) + "”</b>"
                        color: Theme.colorTextPrimary
                        font.pixelSize: Theme.fontSizeMedium
                        elide: Text.ElideRight
                    }

                    Text {
                        Layout.fillWidth: true
                        text: offer.modelData.hint
                        color: Theme.colorTextSecondary
                        font.pixelSize: Theme.fontSizeXSmall
                        elide: Text.ElideRight
                    }
                }

                Rectangle {
                    Layout.preferredWidth: badgeText.implicitWidth + 12
                    Layout.preferredHeight: badgeText.implicitHeight + 4
                    radius: 3
                    color: "transparent"
                    border.width: 1
                    border.color: Theme.colorBorderSubtle

                    Text {
                        id: badgeText
                        anchors.centerIn: parent
                        text: offer.modelData.kind.toUpperCase()
                        color: Theme.colorTextSecondary
                        font.pixelSize: Theme.fontSizeTiny
                        font.weight: Font.DemiBold
                        font.letterSpacing: 0.8
                    }
                }
            }
        }
    }
}
