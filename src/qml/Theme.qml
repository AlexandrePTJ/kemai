pragma Singleton

import QtQuick

QtObject {

    // -- Backgrounds
    readonly property color colorBackground:        "#2d2b3d"
    readonly property color colorBackgroundDark:    "#1e1c2e"
    readonly property color colorSurface:           "#3d3b4d"
    readonly property color colorSurfaceLight:      "#282A35"
    readonly property color colorActiveEntry:       "#1a2a1a"

    // -- Text
    readonly property color colorTextPrimary:       "#ffffff"
    readonly property color colorTextSecondary:     "#9E9E9E"
    readonly property color colorTextTertiary:      "#BCBCBC"

    // -- Accent
    readonly property color colorAccentGreen:         "#4CAF50"
    readonly property color colorAccentGreenTint:     "#194CB04F"
    readonly property color colorAccentGreenHover:    "#a3d7a5"
    readonly property color colorAccentGreenDisabled: "#255627"
    readonly property color colorBorder:              "#1a2a1a"

    readonly property color colorAccentRed:         "#E53935"
    readonly property color colorAccentRedTint:     "#66751919"
    readonly property color colorAccentRedHover:    "#f4a8a6"
    readonly property color colorAccentRedDisabled: "#891412"

    // -- Buttons 
    readonly property color colorButtonStop:        "#B71C1C"
    readonly property color colorButtonStopDown:    "#E53935"
    readonly property color colorButtonStart:       "#388E3C"
    readonly property color colorButtonStartDown:   "#4CAF50"
    readonly property color colorButtonEdit:        "#37474F"
    readonly property color colorButtonEditDown:    "#455A64"
    readonly property color colorButtonPrimary:     "#1565C0"
    readonly property color colorButtonPrimaryDown: "#2196F3"

    // -- Font 
    readonly property int fontSizeXSmall:  11
    readonly property int fontSizeSmall:   12
    readonly property int fontSizeMedium:  13
    readonly property int fontSizeNormal:  14
    readonly property int fontSizeLarge:   15
    readonly property int fontSizeXLarge:  18
    readonly property int fontSizeTimer:   28

    readonly property string fontFamilyMono: "monospace"

    // -- Spacing & Radius
    readonly property int radiusSmall: 4
    readonly property int radiusMedium: 6
    readonly property int radiusLarge: 8
    readonly property int marginsMain: 24
    readonly property int spacingSmall: 6
    readonly property int spacingMedium: 16
    readonly property int spacingLarge: 24
    readonly property int spacingXLarge: 32
}
