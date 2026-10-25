// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kemai

// Account chip opening a menu: Settings…, Log journal…, Check for updates, then Log out (always last)
AbstractButton {
    id: root

    property string accountName: ""
    property string host: ""
    property bool timerRunning: false
    property bool settingsAvailable: false
    // Null: no update entry
    property UpdateNotifier updates: null

    signal logoutRequested
    signal settingsRequested
    signal logJournalRequested

    // Up to two initials from the account name ("KimaiDev" -> "KD", "susan_super" -> "SS")
    function initials(name) {
        const words = name.replace(/([a-z])([A-Z])/g, "$1 $2").split(/[\s._-]+/).filter(w => w.length > 0)
        if (words.length === 0)
            return "?"
        if (words.length === 1)
            return words[0].slice(0, 2).toUpperCase()
        return words[0].charAt(0).toUpperCase() + words[1].charAt(0).toUpperCase()
    }

    // "Last checked today at 08:42"
    function lastCheckedText(when) {
        if (!when || isNaN(when.getTime()))
            return "Never checked"
        const day = new Date(when.getFullYear(), when.getMonth(), when.getDate())
        const today = new Date()
        today.setHours(0, 0, 0, 0)
        const days = Math.round((today - day) / 86400000)
        const time = Qt.formatTime(when, "HH:mm")
        if (days === 0)
            return "Last checked today at " + time
        if (days === 1)
            return "Last checked yesterday at " + time
        return "Last checked on " + Qt.formatDate(when, "d MMM")
    }

    function displayHost(url) {
        return url.replace(/^https?:\/\//, "").replace(/\/+$/, "")
    }

    implicitHeight: 28
    implicitWidth: chipRow.implicitWidth + 11
    focusPolicy: Qt.StrongFocus
    checkable: false

    ToolTip.visible: root.hovered && !menu.opened
    ToolTip.delay: 600
    ToolTip.text: "Account"

    onClicked: menu.opened ? menu.close() : menu.open()

    Keys.onPressed: event => {
        if (event.key === Qt.Key_Down || event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
            menu.open()
            event.accepted = true
        }
    }

    HoverHandler {
        cursorShape: Qt.PointingHandCursor
    }

    component Avatar: Rectangle {
        id: avatar

        property real size: 22

        implicitWidth: avatar.size
        implicitHeight: avatar.size
        radius: avatar.size / 2
        color: Theme.colorAccentGreenSoft

        Text {
            anchors.centerIn: parent
            text: root.initials(root.accountName)
            color: Theme.colorAccentGreen
            font.pixelSize: Math.round(avatar.size * 0.42)
            font.weight: Font.Bold
        }
    }

    background: Rectangle {
        radius: height / 2
        color: menu.opened || root.down ? Theme.colorSurface : Theme.colorSurfaceDark
        border.width: 1
        border.color: menu.opened || root.visualFocus ? Theme.colorAccentGreenDim : Theme.colorBorderSubtle
    }

    contentItem: Item {
        RowLayout {
            id: chipRow
            anchors.verticalCenter: parent.verticalCenter
            x: 3
            spacing: 6

            Avatar {}

            Text {
                text: root.accountName
                color: Theme.colorTextPrimary
                font.pixelSize: Theme.fontSizeSmall
                font.weight: Font.Medium
            }

            Icon {
                name: "chevron-down"
                size: 11
            }
        }
    }

    Popup {
        id: menu

        y: root.height + 6
        x: root.width - menu.width
        width: 270
        padding: 4
        focus: true
        // Pressing the chip itself toggles the menu
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

        onOpened: logoutItem.forceActiveFocus()

        background: Rectangle {
            color: Theme.colorBackground
            radius: Theme.radiusMedium
            border.width: 1
            border.color: Theme.colorBorderSubtle
        }

        contentItem: ColumnLayout {
            spacing: 0

            // Header: who and where (not clickable)
            RowLayout {
                Layout.fillWidth: true
                Layout.margins: 10
                Layout.topMargin: 8
                spacing: 10

                Avatar {
                    size: 28
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 1

                    Text {
                        Layout.fillWidth: true
                        text: root.accountName
                        color: Theme.colorTextPrimary
                        font.pixelSize: Theme.fontSizeMedium
                        font.weight: Font.DemiBold
                        elide: Text.ElideRight
                    }

                    Text {
                        Layout.fillWidth: true
                        text: root.displayHost(root.host)
                        color: Theme.colorTextSecondary
                        font.pixelSize: Theme.fontSizeXSmall
                        elide: Text.ElideRight
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.leftMargin: -4
                Layout.rightMargin: -4
                Layout.bottomMargin: 4
                Layout.preferredHeight: 1
                color: Theme.colorDivider
            }

            MenuEntry {
                id: settingsItem
                objectName: "settingsMenuItem"
                visible: root.settingsAvailable
                iconName: "settings"
                text: "Settings…"
                onClicked: {
                    menu.close()
                    root.settingsRequested()
                }
            }

            MenuEntry {
                objectName: "logJournalMenuItem"
                iconName: "list"
                text: "Log journal…"
                onClicked: {
                    menu.close()
                    root.logJournalRequested()
                }
            }

            MenuEntry {
                objectName: "checkUpdatesMenuItem"
                visible: root.updates !== null && root.updates.enabled && !root.updates.devBuild
                iconName: "refresh-cw"
                text: "Check for updates"
                detail: root.updates !== null && root.updates.preferences ? root.lastCheckedText(root.updates.preferences.lastUpdateCheck) : ""
                onClicked: {
                    menu.close()
                    root.updates.checkNow(UpdateNotifier.Menu)
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.leftMargin: -4
                Layout.rightMargin: -4
                Layout.topMargin: 4
                Layout.bottomMargin: 4
                Layout.preferredHeight: 1
                color: Theme.colorDivider
            }

            MenuEntry {
                id: logoutItem
                objectName: "logoutMenuItem"
                iconName: "log-out"
                text: "Log out"
                detail: root.timerRunning ? "Your timer keeps running on Kimai" : "Auto-login will be turned off"
                detailColor: root.timerRunning ? Theme.colorAccentGreen : Theme.colorTextSecondary
                detailDot: root.timerRunning
                onClicked: {
                    menu.close()
                    root.logoutRequested()
                }
            }
        }
    }
}
