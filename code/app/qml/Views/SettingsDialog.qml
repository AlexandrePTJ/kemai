// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kemai


// Per-computer settings, opened from the account menu and the login view. Every change applies and is saved
// at once: no Save/Cancel. Sections come with their features (SettingsSection), none is built in.
Popup {
    id: root

    required property KemaiContext kemaiContext
    required property PresenceMonitor presence
    property list<SettingsSection> sections: [inactivitySection, certificatesSection]

    // Trusted certificate asking for removal confirmation ("" if none)
    property string _confirmRemoveHost: ""

    onClosed: root._confirmRemoveHost = ""

    SettingsSection {
        id: certificatesSection
        key: "certificates"
        title: "Trusted certificates"
        navLabel: "Certificates"
        description: "Server certificates you chose to trust although they couldn't be verified."
        icon: "shield-check"
        content: Component {
            ColumnLayout {
                spacing: 0

                // Empty state
                Item {
                    visible: root.kemaiContext.trustedCertificates.length === 0
                    Layout.fillWidth: true
                    Layout.topMargin: 10
                    Layout.preferredHeight: emptyColumn.implicitHeight + 56

                    DashedBorder {
                        anchors.fill: parent
                        radius: Theme.radiusMedium
                        color: Theme.colorBorderSubtle
                    }

                    ColumnLayout {
                        id: emptyColumn
                        anchors.centerIn: parent
                        width: Math.min(parent.width - 40, 300)
                        spacing: 6

                        Icon {
                            Layout.alignment: Qt.AlignHCenter
                            name: "shield"
                            size: 22
                            color: Theme.colorTextDim
                        }

                        Text {
                            Layout.alignment: Qt.AlignHCenter
                            Layout.topMargin: 4
                            text: "No trusted certificates"
                            color: Theme.colorTextPrimary
                            font.pixelSize: Theme.fontSizeMedium
                            font.weight: Font.Medium
                        }

                        Text {
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignHCenter
                            text: "When you connect to a server whose certificate can't be verified and choose to trust it, it appears here."
                            color: Theme.colorTextSecondary
                            font.pixelSize: Theme.fontSizeSmall
                            wrapMode: Text.Wrap
                        }
                    }
                }

                // List
                Rectangle {
                    visible: root.kemaiContext.trustedCertificates.length > 0
                    Layout.fillWidth: true
                    Layout.topMargin: 10
                    Layout.preferredHeight: certificateRows.implicitHeight + 2
                    radius: Theme.radiusMedium
                    color: Theme.colorSurfaceDark
                    border.width: 1
                    border.color: Theme.colorBorderSubtle
                    clip: true

                    ColumnLayout {
                        id: certificateRows
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 1
                        spacing: 0

                        Repeater {
                            model: root.kemaiContext.trustedCertificates

                            delegate: ColumnLayout {
                                id: certificateRow

                                required property var modelData
                                required property int index

                                readonly property bool confirming: root._confirmRemoveHost === certificateRow.modelData.host
                                // First and last 3 bytes
                                readonly property string shortSha: {
                                    const bytes = certificateRow.modelData.sha256.split(":")
                                    return bytes.slice(0, 3).join(":") + ":…:" + bytes.slice(-3).join(":")
                                }

                                Layout.fillWidth: true
                                spacing: 0

                                Rectangle {
                                    visible: certificateRow.index > 0
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 1
                                    color: Theme.colorDivider
                                }

                                // Row
                                RowLayout {
                                    visible: !certificateRow.confirming
                                    Layout.fillWidth: true
                                    Layout.margins: 10
                                    Layout.leftMargin: 12
                                    Layout.rightMargin: 12
                                    spacing: 10

                                    Icon {
                                        name: "shield"
                                        size: 15
                                        color: certificateRow.modelData.expired ? Theme.colorAccentRed : Theme.colorTextSecondary
                                    }

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 2

                                        Text {
                                            Layout.fillWidth: true
                                            text: certificateRow.modelData.host
                                            color: Theme.colorTextPrimary
                                            font.pixelSize: Theme.fontSizeMedium
                                            font.weight: Font.Medium
                                            elide: Text.ElideRight
                                        }

                                        RowLayout {
                                            spacing: 10

                                            Text {
                                                text: "SHA-256 " + certificateRow.shortSha
                                                color: Theme.colorTextSecondary
                                                font.family: Theme.fontFamilyMono
                                                font.pixelSize: Theme.fontSizeXSmall

                                                HoverHandler {
                                                    id: shaHover
                                                }

                                                ToolTip.visible: shaHover.hovered
                                                ToolTip.delay: 400
                                                ToolTip.text: certificateRow.modelData.sha256
                                            }

                                            Text {
                                                text: (certificateRow.modelData.expired ? "Expired " : "Expires ")
                                                      + Qt.formatDate(certificateRow.modelData.expiresAt, "d MMM yyyy")
                                                color: certificateRow.modelData.expired ? Theme.colorAccentRed : Theme.colorTextSecondary
                                                font.pixelSize: Theme.fontSizeXSmall
                                                font.features: Theme.fontFeaturesTabular
                                            }
                                        }
                                    }

                                    AbstractButton {
                                        id: removeButton
                                        Layout.preferredWidth: 28
                                        Layout.preferredHeight: 28
                                        focusPolicy: Qt.StrongFocus
                                        onClicked: root._confirmRemoveHost = certificateRow.modelData.host

                                        ToolTip.visible: removeButton.hovered
                                        ToolTip.delay: 500
                                        ToolTip.text: "Stop trusting"

                                        HoverHandler {
                                            cursorShape: Qt.PointingHandCursor
                                        }

                                        background: Rectangle {
                                            radius: Theme.radiusMedium
                                            color: removeButton.hovered ? Theme.colorSurface : "transparent"
                                            border.width: removeButton.visualFocus ? 1 : 0
                                            border.color: Theme.colorAccentGreenDim
                                        }

                                        contentItem: Item {
                                            Icon {
                                                anchors.centerIn: parent
                                                name: "trash-2"
                                                size: 14
                                                color: Theme.colorTextSecondary
                                            }
                                        }
                                    }
                                }

                                // Inline removal confirmation
                                Rectangle {
                                    visible: certificateRow.confirming
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: confirmRow.implicitHeight + 20
                                    color: Theme.colorAccentRedSoft
                                    border.width: 1
                                    border.color: Theme.colorAccentRedLine

                                    Keys.onEscapePressed: root._confirmRemoveHost = ""

                                    RowLayout {
                                        id: confirmRow
                                        anchors.left: parent.left
                                        anchors.right: parent.right
                                        anchors.verticalCenter: parent.verticalCenter
                                        anchors.leftMargin: 12
                                        anchors.rightMargin: 12
                                        spacing: 10

                                        Icon {
                                            name: "trash-2"
                                            size: 15
                                            color: Theme.colorAccentRed
                                        }

                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: 2

                                            Text {
                                                Layout.fillWidth: true
                                                text: "Stop trusting " + certificateRow.modelData.host + "?"
                                                color: Theme.colorTextPrimary
                                                font.pixelSize: Theme.fontSizeMedium
                                                font.weight: Font.DemiBold
                                                elide: Text.ElideRight
                                            }

                                            Text {
                                                Layout.fillWidth: true
                                                text: "Kemai will ask again at the next connection."
                                                color: Theme.colorTextSecondary
                                                font.pixelSize: Theme.fontSizeSmall
                                                elide: Text.ElideRight
                                            }
                                        }

                                        DialogButton {
                                            id: keepButton
                                            text: "Keep"
                                            onClicked: root._confirmRemoveHost = ""
                                            Keys.onEscapePressed: root._confirmRemoveHost = ""
                                        }

                                        DialogButton {
                                            variant: "danger"
                                            text: "Remove"
                                            onClicked: {
                                                root.kemaiContext.removeTrustedCertificate(certificateRow.modelData.host)
                                                root._confirmRemoveHost = ""
                                            }
                                        }
                                    }

                                    // Keep has the focus: removal is a deliberate choice
                                    onVisibleChanged: if (visible) keepButton.forceActiveFocus(Qt.TabFocusReason)
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    SettingsSection {
        id: inactivitySection
        key: "inactivity"
        title: "Inactivity"
        description: "Stop a running timer when you step away."
        icon: "moon"
        content: Component {
            ColumnLayout {
                id: inactivityRows

                readonly property AppPreferences preferences: root.kemaiContext.preferences

                spacing: 0

                SettingsRow {
                    Layout.fillWidth: true
                    title: "Stop the timer when idle"
                    description: "No keyboard or mouse activity for the chosen time. The entry ends when activity stopped."
                    unavailableReason: root.presence.idleSupported ? "" : "This desktop session doesn't report idle time."
                    control: SettingsToggle {
                        objectName: "stopOnIdleToggle"
                        checked: inactivityRows.preferences.stopOnIdle
                        onToggled: inactivityRows.preferences.stopOnIdle = checked
                    }

                    RowLayout {
                        Layout.topMargin: 10
                        spacing: 10
                        enabled: root.presence.idleSupported

                        Text {
                            text: "After"
                            color: root.presence.idleSupported ? Theme.colorTextPrimary : Theme.colorTextDim
                            font.pixelSize: Theme.fontSizeMedium
                        }

                        SettingsStepper {
                            objectName: "idleMinutesStepper"
                            from: AppPreferences.MinIdleMinutes
                            to: AppPreferences.MaxIdleMinutes
                            value: inactivityRows.preferences.idleMinutes
                            onEdited: (minutes) => inactivityRows.preferences.idleMinutes = minutes
                        }

                        Text {
                            text: "minutes"
                            color: root.presence.idleSupported ? Theme.colorTextPrimary : Theme.colorTextDim
                            font.pixelSize: Theme.fontSizeMedium
                        }

                        Text {
                            text: AppPreferences.MinIdleMinutes + " – " + AppPreferences.MaxIdleMinutes
                            color: Theme.colorTextDim
                            font.pixelSize: Theme.fontSizeXSmall
                            font.features: Theme.fontFeaturesTabular
                        }
                    }
                }

                SettingsRow {
                    Layout.fillWidth: true
                    last: true
                    title: "Stop the timer when the session is locked"
                    description: "The entry ends at the moment the screen was locked."
                    unavailableReason: root.presence.lockSupported ? "" : "This desktop session doesn't report screen locking."
                    control: SettingsToggle {
                        objectName: "stopOnLockToggle"
                        checked: inactivityRows.preferences.stopOnLock
                        onToggled: inactivityRows.preferences.stopOnLock = checked
                    }
                }
            }
        }
    }

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
                Layout.fillWidth: true
                text: "Settings"
                color: Theme.colorTextPrimary
                font.pixelSize: Theme.fontSizeNormal
                font.weight: Font.DemiBold
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
                                text: navItem.modelData.navLabel !== "" ? navItem.modelData.navLabel : navItem.modelData.title
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
                        font.pixelSize: Theme.fontSizeTitle
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
