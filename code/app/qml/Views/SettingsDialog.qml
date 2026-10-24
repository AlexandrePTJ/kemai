// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kemai

import "../Components"

// Per-computer settings, opened from the account menu and the login view. Every change applies and is saved
// at once: no Save/Cancel. Sections come with their features (SettingsSection), none is built in.
Popup {
    id: root

    required property KemaiContext kemaiContext
    // Header context: before login the dialog says so
    property bool signedIn: false
    property list<SettingsSection> sections

    readonly property bool hasSections: root.sections.length > 0

    // Last section opened in this run
    property string _currentKey: ""
    readonly property int _currentIndex: Math.max(0, root._indexOf(root._currentKey))
    readonly property SettingsSection _current: root.hasSections ? root.sections[root._currentIndex] : null

    function _indexOf(key) {
        for (let i = 0; i < root.sections.length; ++i) {
            if (root.sections[i].key === key)
                return i
        }
        return -1
    }

    parent: Overlay.overlay
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape
    width: parent ? Math.min(660, parent.width - 40) : 660
    height: parent ? Math.min(500, parent.height - 40) : 500
    x: parent ? (parent.width - width) / 2 : 0
    y: parent ? (parent.height - height) / 2 : 0
    padding: 0

    onOpened: navList.forceActiveFocus()

    Overlay.modal: Rectangle {
        color: Theme.colorBackdrop
    }

    background: Rectangle {
        color: Theme.colorBackground
        border.color: Theme.colorBorderSubtle
        border.width: 1
        radius: Theme.radiusDialog
    }

    contentItem: ColumnLayout {
        spacing: 0

        // -- Header
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 18
            Layout.rightMargin: 18
            Layout.topMargin: 13
            Layout.bottomMargin: 13
            spacing: 10

            Icon {
                name: "settings"
                size: 16
                color: Theme.colorTextSecondary
            }

            Text {
                text: "Settings"
                color: Theme.colorTextPrimary
                font.pixelSize: Theme.fontSizeNormal
                font.weight: Font.DemiBold
            }

            Text {
                Layout.fillWidth: true
                text: root.signedIn ? "This computer" : "This computer · not signed in"
                color: Theme.colorTextSecondary
                font.pixelSize: Theme.fontSizeSmall
                elide: Text.ElideRight
            }

            Icon {
                objectName: "settingsCloseButton"
                name: "x"
                size: 16
                color: Theme.colorTextSecondary

                TapHandler {
                    onTapped: root.close()
                }

                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: Theme.colorDivider
        }

        // -- Settings file unreadable: nothing is written
        Rectangle {
            objectName: "sessionOnlyNotice"
            visible: !root.kemaiContext.settingsPersistent
            Layout.fillWidth: true
            Layout.preferredHeight: noticeRow.implicitHeight + 20
            color: Theme.colorWarningSoft

            RowLayout {
                id: noticeRow
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: 18
                anchors.rightMargin: 18
                spacing: 10

                Icon {
                    Layout.alignment: Qt.AlignTop
                    Layout.topMargin: 1
                    name: "circle-alert"
                    size: 15
                    color: Theme.colorWarning
                }

                Text {
                    Layout.fillWidth: true
                    textFormat: Text.StyledText
                    text: "<b>Changes are kept for this session only.</b> <font color=\"" + Theme.colorTextSecondary
                          + "\">Kemai couldn't read its settings file, so it won't overwrite it.</font>"
                    color: Theme.colorTextPrimary
                    font.pixelSize: Theme.fontSizeSmall
                    // 12 px at 1.45
                    lineHeightMode: Text.FixedHeight
                    lineHeight: 17
                    wrapMode: Text.Wrap
                }
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                color: Theme.colorWarningLine
            }
        }

        // -- Navigation | content
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            Rectangle {
                Layout.preferredWidth: 170
                Layout.fillHeight: true
                color: Theme.colorBackgroundDark
                // Square under the dialog's rounded bottom-left corner: clipped by a radius on the outer corner only
                bottomLeftRadius: Theme.radiusDialog

                ListView {
                    id: navList
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 2
                    interactive: false
                    keyNavigationEnabled: true
                    currentIndex: root._currentIndex
                    model: root.sections
                    // Tab goes on to the content
                    activeFocusOnTab: true

                    onCurrentIndexChanged: {
                        if (navList.currentIndex >= 0 && navList.currentIndex < root.sections.length)
                            root._currentKey = root.sections[navList.currentIndex].key
                    }

                    delegate: AbstractButton {
                        id: navItem

                        required property SettingsSection modelData
                        required property int index

                        readonly property bool active: navItem.index === root._currentIndex

                        width: ListView.view.width
                        implicitHeight: 34
                        focusPolicy: Qt.NoFocus
                        onClicked: root._currentKey = navItem.modelData.key

                        HoverHandler {
                            cursorShape: Qt.PointingHandCursor
                        }

                        background: Rectangle {
                            radius: Theme.radiusMedium
                            color: navItem.active ? Theme.colorSurface : "transparent"

                            // Accent bar at the column's left edge
                            Rectangle {
                                visible: navItem.active
                                x: -8
                                y: 8
                                width: 2
                                height: parent.height - 16
                                radius: 1
                                color: Theme.colorAccentGreen
                            }
                        }

                        contentItem: RowLayout {
                            spacing: 10

                            Icon {
                                Layout.leftMargin: 10
                                name: navItem.modelData.icon
                                size: 15
                                color: navItem.active ? Theme.colorAccentGreen : Theme.colorTextSecondary
                            }

                            Text {
                                Layout.fillWidth: true
                                text: navItem.modelData.title
                                color: navItem.active ? Theme.colorTextPrimary : Theme.colorTextSecondary
                                font.pixelSize: Theme.fontSizeMedium
                                font.weight: navItem.active ? Font.Medium : Font.Normal
                                elide: Text.ElideRight
                            }
                        }
                    }
                }

                Rectangle {
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                    width: 1
                    color: Theme.colorDivider
                }
            }

            Flickable {
                id: contentFlick
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                contentWidth: width
                contentHeight: contentColumn.implicitHeight + 36
                boundsBehavior: Flickable.StopAtBounds

                WheelScroller {
                    flickable: contentFlick
                }

                ScrollBar.vertical: ScrollBar {}

                ColumnLayout {
                    id: contentColumn
                    x: 22
                    y: 18
                    width: contentFlick.width - 44
                    spacing: 0

                    Text {
                        Layout.fillWidth: true
                        text: root._current ? root._current.title : ""
                        color: Theme.colorTextPrimary
                        font.pixelSize: 16
                        font.weight: Font.DemiBold
                    }

                    Text {
                        visible: text !== ""
                        Layout.fillWidth: true
                        Layout.topMargin: 3
                        Layout.bottomMargin: 4
                        text: root._current ? root._current.description : ""
                        color: Theme.colorTextSecondary
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.Wrap
                    }

                    Loader {
                        Layout.fillWidth: true
                        sourceComponent: root._current ? root._current.content : null
                        onLoaded: contentFlick.contentY = 0
                    }
                }
            }
        }
    }
}
