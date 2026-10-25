// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kemai

// Full-width band telling that a newer Kemai exists, or the result of a check started from a menu. Unlike the
// notification banner it stays until the user acts or dismisses it (up to date and "ignored" hide after 6 s).
// Never installs anything: "See the release" opens the release page.
Rectangle {
    id: root

    required property UpdateNotifier notifier

    // Version the user just chose to ignore: the "ignored" state, with Undo
    property string _ignored: ""
    property string _previouslyIgnored: ""

    readonly property string _state: {
        if (!root.notifier.enabled)
            return ""
        if (root._ignored !== "")
            return "ignored"
        if (root.notifier.source === UpdateNotifier.Settings)
            return ""
        switch (root.notifier.status) {
        case UpdateNotifier.Available:
            return "available"
        case UpdateNotifier.Checking:
            return "checking"
        case UpdateNotifier.UpToDate:
            return "upToDate"
        case UpdateNotifier.Failed:
            return "failed"
        }
        return ""
    }

    // "2026.12.1 alpha": the channel only when it isn't stable
    readonly property string _latest: root.notifier.latestVersion + (root.notifier.latestPrerelease ? " alpha" : "")
    readonly property string _current: root.notifier.currentVersion + (root.notifier.channel === "alpha" ? " alpha" : "")
    readonly property string _reason: {
        switch (root.notifier.failure) {
        case UpdateNotifier.NoNetwork:
            return "no connection to github.com"
        case UpdateNotifier.ServerError:
            return "GitHub didn't answer (server error)"
        }
        return "unexpected response"
    }

    function _ignore() {
        const preferences = root.notifier.preferences
        root._previouslyIgnored = preferences.ignoredVersion
        preferences.ignoredVersion = root.notifier.latestVersion
        root._ignored = root.notifier.latestVersion
        autoHide.restart()
    }

    function _undoIgnore() {
        root.notifier.preferences.ignoredVersion = root._previouslyIgnored
        root._ignored = ""
        autoHide.stop()
    }

    visible: root._state !== ""
    implicitHeight: Math.max(46, stripRow.implicitHeight + 16)
    color: Theme.colorSurfaceDark

    on_StateChanged: {
        if (root._state === "upToDate")
            autoHide.restart()
        spinnerDelay.restart()
    }

    // Up to date and ignored hide themselves
    Timer {
        id: autoHide
        interval: 6000
        onTriggered: {
            if (root._ignored !== "") {
                root._ignored = ""
                root.notifier.reset()
            } else if (root._state === "upToDate") {
                root.notifier.reset()
            }
        }
    }

    // The spinner only for checks that take a while
    Timer {
        id: spinnerDelay
        interval: 150
    }

    // Rail: accent for a new version, danger for a failure
    Rectangle {
        visible: root._state === "available" || root._state === "failed"
        width: 3
        height: parent.height
        color: root._state === "failed" ? Theme.colorAccentRed : Theme.colorAccentGreen
    }

    Rectangle {
        anchors.bottom: parent.bottom
        width: parent.width
        height: 1
        color: Theme.colorDivider
    }

    component IconButton: AbstractButton {
        id: iconButton

        property string iconName
        property string tip

        Layout.preferredWidth: 26
        Layout.preferredHeight: 26
        focusPolicy: Qt.StrongFocus

        ToolTip.visible: iconButton.hovered
        ToolTip.delay: 500
        ToolTip.text: iconButton.tip

        HoverHandler {
            cursorShape: Qt.PointingHandCursor
        }

        background: Rectangle {
            radius: Theme.radiusMedium
            color: iconButton.hovered || iconButton.visualFocus ? Theme.colorSurface : "transparent"
        }

        contentItem: Item {
            Icon {
                anchors.centerIn: parent
                name: iconButton.iconName
                size: 14
                color: iconButton.hovered ? Theme.colorTextPrimary : Theme.colorTextSecondary
            }
        }
    }

    component TextButton: AbstractButton {
        id: textButton

        Layout.preferredHeight: 26
        padding: 6
        focusPolicy: Qt.StrongFocus

        HoverHandler {
            cursorShape: Qt.PointingHandCursor
        }

        background: Rectangle {
            radius: Theme.radiusMedium
            color: textButton.hovered || textButton.visualFocus ? Theme.colorSurface : "transparent"
        }

        contentItem: Text {
            text: textButton.text
            color: textButton.hovered ? Theme.colorTextPrimary : Theme.colorTextSecondary
            font.pixelSize: Theme.fontSizeSmall
            font.weight: Font.Medium
            verticalAlignment: Text.AlignVCenter
        }
    }

    RowLayout {
        id: stripRow
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 10
        anchors.topMargin: 8
        anchors.bottomMargin: 8
        spacing: 10

        Rectangle {
            Layout.preferredWidth: 26
            Layout.preferredHeight: 26
            radius: Theme.radiusMedium
            color: root._state === "failed" ? Theme.colorAccentRedSoft : root._state === "available" || root._state === "upToDate" ? Theme.colorAccentGreenSoft : Theme.colorSurface

            Icon {
                anchors.centerIn: parent
                size: 14
                name: {
                    switch (root._state) {
                    case "available":
                        return "circle-arrow-up"
                    case "checking":
                        return "refresh-cw"
                    case "upToDate":
                        return "check"
                    case "failed":
                        return "circle-alert"
                    }
                    return "bell-off"
                }
                color: root._state === "failed" ? Theme.colorAccentRed : root._state === "available" || root._state === "upToDate" ? Theme.colorAccentGreen : Theme.colorTextSecondary
            }
        }

        // Title and detail
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 1

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Spinner {
                    visible: root._state === "checking" && !spinnerDelay.running
                    size: 13
                    color: Theme.colorAccentGreen
                    trackColor: Theme.colorBorderSubtle
                }

                Text {
                    Layout.fillWidth: true
                    textFormat: Text.StyledText
                    text: {
                        switch (root._state) {
                        case "available":
                            return "<b>Kemai " + root._latest + " is available</b>"
                        case "checking":
                            return "Checking for updates…"
                        case "upToDate":
                            return "<b>Kemai is up to date</b> · " + root._current + " is the latest " + (root.notifier.channel === "stable" ? "release." : "alpha.")
                        case "failed":
                            return "<b>Couldn't check for updates</b> — " + root._reason
                        case "ignored":
                            return "<b>" + root._ignored + " won't be announced again.</b> You can undo this in Settings › Updates."
                        }
                        return ""
                    }
                    color: Theme.colorTextPrimary
                    font.pixelSize: Theme.fontSizeMedium
                    font.features: Theme.fontFeaturesTabular
                    elide: Text.ElideRight
                }
            }

            Text {
                visible: root._state === "available" || root._state === "failed"
                Layout.fillWidth: true
                text: root._state === "available" ? "You have " + root._current + ". Release notes and build instructions are on GitHub." : root.notifier.preferences && root.notifier.preferences.checkUpdatesAtStart && !root.notifier.devBuild ? "Kemai will try again at next start." : "Try again later."
                color: Theme.colorTextSecondary
                font.pixelSize: Theme.fontSizeSmall
                font.features: Theme.fontFeaturesTabular
                elide: Text.ElideRight
            }
        }

        TextButton {
            visible: root._state === "available"
            text: "Ignore this version"
            onClicked: root._ignore()
        }

        TextButton {
            visible: root._state === "ignored"
            text: "Undo"
            onClicked: root._undoIgnore()
        }

        DialogButton {
            visible: root._state === "available"
            implicitHeight: 28
            variant: "primary"
            iconName: "external-link"
            text: "See the release"
            onClicked: Qt.openUrlExternally(root.notifier.releaseUrl)
        }

        DialogButton {
            visible: root._state === "failed"
            implicitHeight: 28
            iconName: "refresh-cw"
            text: "Retry"
            onClicked: root.notifier.checkNow(UpdateNotifier.Menu)
        }

        IconButton {
            iconName: "x"
            tip: root._state === "available" ? "Dismiss for now — Kemai will tell you again at next start" : "Close"
            onClicked: {
                root._ignored = ""
                autoHide.stop()
                root.notifier.reset()
            }
        }
    }
}
