// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Shapes

import "LucideIcons.js" as LucideIcons

// Lucide line icon by name, drawn as a Shape so it takes any color at any size
Item {
    id: root

    property string name
    property color color: Theme.colorTextSecondary
    property real size: 16

    implicitWidth: root.size
    implicitHeight: root.size

    Shape {
        anchors.fill: parent
        preferredRendererType: Shape.CurveRenderer

        ShapePath {
            scale: Qt.size(root.size / 24, root.size / 24)
            strokeColor: root.color
            // Lucide's 24-unit grid with the handoff's 1.75 stroke
            strokeWidth: 1.75 * root.size / 24
            fillColor: "transparent"
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin

            PathSvg {
                path: LucideIcons.paths[root.name] ?? ""
            }
        }
    }
}
