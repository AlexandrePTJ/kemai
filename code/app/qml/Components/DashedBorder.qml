// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Shapes

import Kemai

// Dashed rounded outline filling its parent (Rectangle borders are solid only)
Shape {
    id: root

    property color color: Theme.colorBorderSubtle
    property real radius: Theme.radiusMedium

    anchors.fill: parent
    preferredRendererType: Shape.CurveRenderer

    ShapePath {
        strokeColor: root.color
        strokeWidth: 1
        strokeStyle: ShapePath.DashLine
        dashPattern: [3, 3]
        fillColor: "transparent"

        PathRectangle {
            x: 0.5
            y: 0.5
            width: root.width - 1
            height: root.height - 1
            radius: root.radius
        }
    }
}
