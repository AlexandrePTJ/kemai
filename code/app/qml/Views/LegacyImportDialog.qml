// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kemai

// Offers to import the profiles of Kemai 0.11 found on this computer, all at once (content:
// openspec/changes/legacy-profile-import/screens/profile-import.md). Escape answers Later.
Popup {
    id: root

    required property ProfileManager profileManager

    // At least one profile saved: the login view selects 0.11's last one
    signal imported
    // Don't ask again
    signal declined

    property bool _busy: false
    property bool _spinnerShown: false
    // name -> reason, from the last import
    property var _failures: ({})
    property int _importedCount: 0

    readonly property var _toImport: root.profileManager.legacyCandidates.filter(c => c.state === "ready")
    readonly property var _noToken: root.profileManager.legacyCandidates.filter(c => c.state === "noToken")
    readonly property bool _failed: Object.keys(root._failures).length > 0

    function _import() {
        if (root._busy || root._toImport.length === 0)
            return
        root._busy = true
        spinnerDelay.restart()
        root.profileManager.importLegacyProfiles()
    }

    parent: Overlay.overlay
    modal: true
    focus: true
    closePolicy: root._busy ? Popup.NoAutoClose : Popup.CloseOnEscape
    width: 460
    height: dialogColumn.implicitHeight
    x: parent ? (parent.width - width) / 2 : 0
    y: parent ? (parent.height - height) / 2 : 0
    padding: 0

    onOpened: importButton.forceActiveFocus(Qt.TabFocusReason)

    Connections {
        target: root.profileManager
        enabled: root._busy
        function onLegacyImportCompleted(importedCount, failures) {
            root._busy = false
            root._spinnerShown = false
            spinnerDelay.stop()
            const byName = {}
            for (const failure of failures)
                byName[failure.name] = failure.reason
            root._failures = byName
            root._importedCount += importedCount
            if (failures.length === 0)
                root.close()
            if (importedCount > 0)
                root.imported()
        }
    }

    Timer {
        id: spinnerDelay
        interval: 150
        onTriggered: root._spinnerShown = true
    }

    Overlay.modal: Rectangle {
        color: Theme.colorBackdrop
    }

    background: Rectangle {
        color: Theme.colorBackground
        border.color: Theme.colorBorderSubtle
        border.width: 1
        radius: Theme.radiusDialog
    }

    ColumnLayout {
        id: dialogColumn
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 0

        Text {
            Layout.fillWidth: true
            Layout.leftMargin: 18
            Layout.rightMargin: 18
            Layout.topMargin: 16
            text: "Import your Kemai 0.11 profiles?"
            color: Theme.colorTextPrimary
            font.pixelSize: Theme.fontSizeNormal
            font.weight: Font.DemiBold
            wrapMode: Text.Wrap
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.margins: 18
            Layout.topMargin: 14
            spacing: 12

            Text {
                Layout.fillWidth: true
                text: root._failed ? (root._importedCount > 0 ? "Some profiles could not be saved. The others are imported." : "These profiles could not be saved.") : "Kemai 0.11 is installed on this computer. Its connections can be imported here, with their API tokens, which Kemai keeps in the system keychain. Kemai 0.11's own settings are left as they are."
                color: Theme.colorTextPrimary
                font.pixelSize: Theme.fontSizeMedium
                lineHeight: 1.45
                wrapMode: Text.Wrap
            }

            // Profiles to import (after a failure: the ones still to save)
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: Math.min(importList.contentHeight, 220)
                visible: root._toImport.length > 0
                radius: Theme.radiusMedium
                color: Theme.colorSurfaceDark
                border.width: 1
                border.color: Theme.colorBorderSubtle
                clip: true

                ListView {
                    id: importList
                    anchors.fill: parent
                    model: root._toImport
                    boundsBehavior: Flickable.StopAtBounds

                    WheelScroller {
                        flickable: importList
                    }

                    delegate: Item {
                        id: row

                        required property var modelData
                        required property int index

                        readonly property string _reason: root._failures[row.modelData.finalName] ?? ""

                        width: ListView.view.width
                        height: rowColumn.implicitHeight + 16

                        Rectangle {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.top: parent.top
                            height: 1
                            visible: row.index > 0
                            color: Theme.colorDivider
                        }

                        ColumnLayout {
                            id: rowColumn
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            spacing: 2

                            Text {
                                id: nameText
                                Layout.fillWidth: true
                                text: row.modelData.finalName
                                textFormat: Text.PlainText
                                color: Theme.colorTextPrimary
                                font.pixelSize: Theme.fontSizeMedium
                                font.weight: Font.DemiBold
                                elide: Text.ElideRight
                            }

                            Text {
                                id: hostText
                                Layout.fillWidth: true
                                text: row.modelData.host
                                textFormat: Text.PlainText
                                color: Theme.colorTextSecondary
                                font.pixelSize: Theme.fontSizeSmall
                                elide: Text.ElideMiddle
                            }

                            Text {
                                Layout.fillWidth: true
                                visible: row._reason !== ""
                                text: "Not saved: " + row._reason
                                textFormat: Text.PlainText
                                color: Theme.colorAccentRed
                                font.pixelSize: Theme.fontSizeSmall
                                wrapMode: Text.Wrap
                            }
                        }

                        HoverHandler {
                            id: rowHover
                        }

                        ToolTip.visible: rowHover.hovered && (nameText.truncated || hostText.truncated)
                        ToolTip.text: row.modelData.finalName + " — " + row.modelData.host
                        ToolTip.delay: 600
                    }
                }
            }

            // Profiles 0.11 kept with the legacy API password only
            Text {
                Layout.fillWidth: true
                visible: root._noToken.length > 0
                text: "Not imported: no API token. Create one in Kimai (your user menu › API access), then add the profile here: " + root._noToken.map(c => c.name).join(", ") + "."
                textFormat: Text.PlainText
                color: Theme.colorTextSecondary
                font.pixelSize: Theme.fontSizeSmall
                wrapMode: Text.Wrap
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: Theme.colorDivider
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.margins: 18
            Layout.topMargin: 12
            Layout.bottomMargin: 12
            spacing: 8

            DialogButton {
                text: "Don't ask again"
                enabled: !root._busy
                onClicked: {
                    root.close()
                    root.declined()
                }
            }

            Item {
                Layout.fillWidth: true
            }

            DialogButton {
                text: "Later"
                enabled: !root._busy
                onClicked: root.close()
            }

            DialogButton {
                id: importButton
                variant: "primary"
                iconName: root._failed ? "refresh-cw" : "download"
                text: root._failed ? "Retry" : "Import"
                busy: root._busy
                spinning: root._spinnerShown
                onClicked: root._import()
                Keys.onReturnPressed: root._import()
                Keys.onEnterPressed: root._import()
            }
        }
    }
}
