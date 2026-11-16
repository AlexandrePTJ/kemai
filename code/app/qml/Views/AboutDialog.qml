// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kemai

import "ThirdParty.js" as ThirdParty

// Which Kemai runs (for bug reports: the version block is selectable, there is no copy button), its license,
// where to get help, and the third-party components on a second view. Esc on that view goes back.
Popup {
    id: root

    // False in tests: links are logged, not opened
    property bool openLinks: true

    property string _view: "main"

    function _open(url) {
        console.info("Opening " + url)
        if (root.openLinks)
            Qt.openUrlExternally(url)
    }

    // "github.com/AlexandrePTJ/kemai"
    function _displayUrl(url) {
        return url.toString().replace(/^https?:\/\/(www\.)?/, "").replace(/\/+$/, "")
    }

    parent: Overlay.overlay
    modal: true
    focus: true
    closePolicy: root._view === "main" ? Popup.CloseOnEscape : Popup.NoAutoClose
    width: Math.min(520, parent ? parent.width - 40 : 520)
    height: Math.min(aboutColumn.implicitHeight, parent ? parent.height - 40 : 600)
    x: parent ? (parent.width - width) / 2 : 0
    y: parent ? (parent.height - height) / 2 : 0
    padding: 0

    onAboutToShow: root._view = "main"
    onOpened: closeButton.forceActiveFocus(Qt.TabFocusReason)

    BuildInfo {
        id: build
    }

    // Esc on the components view goes back to the main one
    Shortcut {
        sequences: [StandardKey.Cancel]
        enabled: root.opened && root._view === "thirdParty"
        onActivated: root._view = "main"
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

    component SectionLabel: Text {
        color: Theme.colorTextLabel
        font.pixelSize: Theme.fontSizeXSmall
        font.weight: Font.DemiBold
        font.letterSpacing: 1.3
        font.capitalization: Font.AllUppercase
    }

    component Box: Rectangle {
        Layout.fillWidth: true
        radius: Theme.radiusMedium
        color: Theme.colorBackgroundDark
        border.width: 1
        border.color: Theme.colorBorderSubtle
    }

    // Selectable value of the version block
    component Value: TextEdit {
        readOnly: true
        selectByMouse: true
        color: Theme.colorTextPrimary
        selectionColor: Theme.colorAccentGreenDim
        selectedTextColor: Theme.colorTextPrimary
        font.pixelSize: Theme.fontSizeSmall
        font.features: Theme.fontFeaturesTabular
    }

    // A row opening a link: icon, label (and license tag), URL, external-link mark
    component LinkRow: AbstractButton {
        id: linkRow

        property string iconName: ""
        property string license: ""
        property url url
        property bool first: false

        Layout.fillWidth: true
        Layout.preferredHeight: 36
        leftPadding: 12
        rightPadding: 12
        focusPolicy: Qt.StrongFocus
        onClicked: root._open(linkRow.url)

        Keys.onReturnPressed: linkRow.clicked()
        Keys.onEnterPressed: linkRow.clicked()

        HoverHandler {
            cursorShape: Qt.PointingHandCursor
        }

        background: Rectangle {
            color: linkRow.hovered || linkRow.visualFocus ? Theme.colorSurface : "transparent"
            radius: Theme.radiusMedium

            // Hairline between rows
            Rectangle {
                visible: !linkRow.first
                width: parent.width
                height: 1
                color: Theme.colorDivider
            }
        }

        contentItem: RowLayout {
            spacing: 10

            Icon {
                visible: linkRow.iconName !== ""
                name: linkRow.iconName
                size: 14
                color: Theme.colorTextSecondary
            }

            Text {
                text: linkRow.text
                color: Theme.colorTextPrimary
                font.pixelSize: Theme.fontSizeMedium
                font.weight: Font.Medium
            }

            Rectangle {
                visible: linkRow.license !== ""
                Layout.preferredWidth: licenseText.implicitWidth + 10
                Layout.preferredHeight: licenseText.implicitHeight + 2
                radius: 3
                color: "transparent"
                border.width: 1
                border.color: Theme.colorBorderSubtle

                Text {
                    id: licenseText
                    anchors.centerIn: parent
                    text: linkRow.license
                    color: Theme.colorTextSecondary
                    font.family: Theme.fontFamilyMono
                    font.pixelSize: Theme.fontSizeTiny
                }
            }

            Text {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignRight
                text: root._displayUrl(linkRow.url)
                color: Theme.colorTextSecondary
                font.pixelSize: Theme.fontSizeSmall
                elide: Text.ElideRight
            }

            Icon {
                name: "external-link"
                size: 12
                color: linkRow.hovered ? Theme.colorAccentGreen : Theme.colorTextDim
            }
        }
    }

    contentItem: ColumnLayout {
        id: aboutColumn
        spacing: 0

        // -- Header: title, or the breadcrumb of the components view
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 18
            Layout.rightMargin: 12
            Layout.topMargin: 13
            Layout.bottomMargin: 13
            spacing: 8

            AbstractButton {
                id: backCrumb
                visible: root._view === "thirdParty"
                Layout.preferredWidth: 22
                Layout.preferredHeight: 22
                focusPolicy: Qt.NoFocus
                onClicked: root._view = "main"

                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }

                background: Rectangle {
                    radius: Theme.radiusSmall
                    color: backCrumb.hovered ? Theme.colorSurface : "transparent"
                }

                contentItem: Item {
                    Icon {
                        anchors.centerIn: parent
                        name: "chevron-left"
                        size: 14
                        color: Theme.colorTextSecondary
                    }
                }
            }

            Text {
                text: "About Kemai"
                color: root._view === "main" ? Theme.colorTextPrimary : Theme.colorTextSecondary
                font.pixelSize: Theme.fontSizeNormal
                font.weight: root._view === "main" ? Font.DemiBold : Font.Normal
            }

            Text {
                visible: root._view === "thirdParty"
                text: "/"
                color: Theme.colorTextDim
                font.pixelSize: Theme.fontSizeNormal
            }

            Text {
                visible: root._view === "thirdParty"
                text: "Third-party components"
                color: Theme.colorTextPrimary
                font.pixelSize: Theme.fontSizeNormal
                font.weight: Font.DemiBold
            }

            Item {
                Layout.fillWidth: true
            }

            AbstractButton {
                id: closeCross
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                focusPolicy: Qt.NoFocus
                onClicked: root.close()

                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }

                background: Rectangle {
                    radius: Theme.radiusSmall
                    color: closeCross.hovered ? Theme.colorSurface : "transparent"
                }

                contentItem: Item {
                    Icon {
                        anchors.centerIn: parent
                        name: "x"
                        size: 16
                        color: closeCross.hovered ? Theme.colorTextPrimary : Theme.colorTextSecondary
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: Theme.colorDivider
        }

        // -- Body (scrolls when the window is short)
        Flickable {
            id: bodyFlick
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: bodyColumn.implicitHeight + 36
            clip: true
            contentWidth: width
            contentHeight: bodyColumn.implicitHeight + 36
            boundsBehavior: Flickable.StopAtBounds
            interactive: contentHeight > height

            WheelScroller {
                flickable: bodyFlick
            }

            ColumnLayout {
                id: bodyColumn
                x: 18
                y: 18
                width: bodyFlick.width - 36
                spacing: 16

                // -- Main view
                // Identity
                RowLayout {
                    visible: root._view === "main"
                    spacing: 14

                    Image {
                        Layout.preferredWidth: 48
                        Layout.preferredHeight: 48
                        source: "qrc:/assets/images/kimai_logo.svg"
                        sourceSize: Qt.size(48, 48)
                        fillMode: Image.PreserveAspectFit
                    }

                    ColumnLayout {
                        spacing: 2

                        Text {
                            text: "Kemai"
                            color: Theme.colorTextPrimary
                            font.pixelSize: 20
                            font.weight: Font.Bold
                        }

                        Text {
                            text: "Desktop client for Kimai"
                            color: Theme.colorTextSecondary
                            font.pixelSize: Theme.fontSizeSmall
                        }
                    }
                }

                // Version
                ColumnLayout {
                    visible: root._view === "main"
                    Layout.fillWidth: true
                    spacing: 8

                    SectionLabel {
                        text: "Version"
                    }

                    Box {
                        Layout.preferredHeight: versionGrid.implicitHeight + 20

                        GridLayout {
                            id: versionGrid
                            anchors.fill: parent
                            anchors.margins: 10
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            columns: 2
                            columnSpacing: 0
                            rowSpacing: 5

                            Label {
                                text: "Kemai"
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 8

                                Value {
                                    objectName: "aboutVersion"
                                    text: build.version
                                    font.pixelSize: Theme.fontSizeMedium
                                    font.weight: Font.DemiBold
                                }

                                // Channel badge; a development build stands out in amber
                                Rectangle {
                                    visible: build.channel !== "stable"
                                    Layout.preferredWidth: channelText.implicitWidth + 14
                                    Layout.preferredHeight: channelText.implicitHeight + 2
                                    radius: height / 2
                                    color: build.channel === "dev" ? Theme.colorWarningSoft : "transparent"
                                    border.width: 1
                                    border.color: build.channel === "dev" ? Theme.colorWarningLine : Theme.colorAccentGreenDim

                                    Text {
                                        id: channelText
                                        anchors.centerIn: parent
                                        text: build.channel === "dev" ? "development build" : build.channel
                                        color: build.channel === "dev" ? Theme.colorWarning : Theme.colorAccentGreen
                                        font.pixelSize: Theme.fontSizeTiny
                                        font.weight: Font.DemiBold
                                        font.capitalization: Font.AllUppercase
                                    }
                                }

                                Item {
                                    Layout.fillWidth: true
                                }
                            }

                            Label {
                                text: "Qt"
                            }

                            Value {
                                objectName: "aboutQtVersion"
                                Layout.fillWidth: true
                                text: build.qtVersion
                            }

                            Label {
                                text: "System"
                            }

                            Value {
                                objectName: "aboutSystem"
                                Layout.fillWidth: true
                                text: build.system
                            }
                        }
                    }
                }

                // Help
                ColumnLayout {
                    visible: root._view === "main"
                    Layout.fillWidth: true
                    spacing: 8

                    SectionLabel {
                        text: "Help"
                    }

                    Box {
                        Layout.preferredHeight: helpColumn.implicitHeight + 2

                        ColumnLayout {
                            id: helpColumn
                            anchors.fill: parent
                            anchors.margins: 1
                            spacing: 0

                            LinkRow {
                                objectName: "sourceLink"
                                first: true
                                iconName: "github"
                                text: "Source code"
                                url: build.sourceUrl
                            }

                            LinkRow {
                                objectName: "issuesLink"
                                iconName: "bug"
                                text: "Report an issue"
                                url: build.issuesUrl
                            }

                            LinkRow {
                                objectName: "kimaiLink"
                                iconName: "globe"
                                text: "Kimai website"
                                url: "https://www.kimai.org"
                            }
                        }
                    }
                }

                // License
                RowLayout {
                    visible: root._view === "main"
                    Layout.fillWidth: true
                    spacing: 7

                    Icon {
                        name: "scale"
                        size: 13
                        color: Theme.colorTextSecondary
                    }

                    Text {
                        Layout.fillWidth: true
                        textFormat: Text.StyledText
                        text: "<font color=\"" + Theme.colorTextPrimary + "\">MIT License</font> · © 2020–2026 Alexandre Petitjean"
                        color: Theme.colorTextSecondary
                        font.pixelSize: Theme.fontSizeSmall
                        elide: Text.ElideRight
                    }

                    AbstractButton {
                        id: thirdPartyLink
                        objectName: "thirdPartyLink"
                        focusPolicy: Qt.StrongFocus
                        onClicked: root._view = "thirdParty"

                        Keys.onReturnPressed: thirdPartyLink.clicked()
                        Keys.onEnterPressed: thirdPartyLink.clicked()

                        HoverHandler {
                            cursorShape: Qt.PointingHandCursor
                        }

                        background: Item {}

                        contentItem: RowLayout {
                            spacing: 3

                            Text {
                                text: "Third-party components"
                                color: Theme.colorAccentGreen
                                font.pixelSize: Theme.fontSizeSmall
                                font.weight: Font.Medium
                                font.underline: thirdPartyLink.hovered || thirdPartyLink.visualFocus
                            }

                            Icon {
                                name: "chevron-right"
                                size: 12
                                color: Theme.colorAccentGreen
                            }
                        }
                    }
                }

                // -- Third-party components view
                Text {
                    visible: root._view === "thirdParty"
                    Layout.fillWidth: true
                    text: "Kemai is built with these components. Each link opens its project page and license."
                    color: Theme.colorTextSecondary
                    font.pixelSize: Theme.fontSizeSmall
                    wrapMode: Text.Wrap
                }

                Box {
                    visible: root._view === "thirdParty"
                    Layout.preferredHeight: componentsColumn.implicitHeight + 2

                    ColumnLayout {
                        id: componentsColumn
                        anchors.fill: parent
                        anchors.margins: 1
                        spacing: 0

                        Repeater {
                            model: ThirdParty.components

                            delegate: LinkRow {
                                required property var modelData
                                required property int index

                                first: index === 0
                                text: modelData.name
                                license: modelData.license
                                url: modelData.url
                            }
                        }
                    }
                }
            }
        }

        // -- Footer
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: Theme.colorDivider
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 18
            Layout.rightMargin: 18
            Layout.topMargin: 12
            Layout.bottomMargin: 12
            spacing: 8

            DialogButton {
                visible: root._view === "thirdParty"
                variant: "ghost"
                iconName: "chevron-left"
                text: "Back"
                onClicked: root._view = "main"
            }

            Item {
                Layout.fillWidth: true
            }

            DialogButton {
                id: closeButton
                text: "Close"
                onClicked: root.close()
                Keys.onReturnPressed: root.close()
                Keys.onEnterPressed: root.close()
            }
        }
    }

    component Label: Text {
        Layout.preferredWidth: 64
        color: Theme.colorTextSecondary
        font.pixelSize: Theme.fontSizeSmall
    }
}
