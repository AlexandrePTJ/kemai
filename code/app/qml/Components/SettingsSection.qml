// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick

// A section of the settings dialog: its navigation entry and its rows (a Component, created when shown).
// Feature changes add theirs to SettingsDialog.sections.
QtObject {
    required property string key
    required property string title
    // Shorter label for the navigation column, when the title is too long for it
    property string navLabel: ""
    property string description: ""
    // Lucide icon name
    required property string icon
    required property Component content
}
