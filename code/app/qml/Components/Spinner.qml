// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Shapes

import Kemai

// Small busy ring: a quarter arc spinning over a faint track
Item {
    id: root

    property real size: 12
    property color color: "white"
    property color trackColor: Qt.rgba(1, 1, 1, 0.3)

    implicitWidth: root.size
    implicitHeight: root.size

    Rectangle {
        anchors.fill: parent
        radius: width / 2
        color: "transparent"
        border.width: 2
        border.color: root.trackColor
    }

    Shape {
        anchors.fill: parent
        preferredRendererType: Shape.CurveRenderer

        ShapePath {
            strokeColor: root.color
            strokeWidth: 2
            fillColor: "transparent"
            capStyle: ShapePath.FlatCap

            PathAngleArc {
                centerX: root.size / 2
                centerY: root.size / 2
                radiusX: root.size / 2 - 1
                radiusY: root.size / 2 - 1
                startAngle: -90
                sweepAngle: 90
            }
        }

        RotationAnimator on rotation {
            from: 0
            to: 360
            duration: 700
            loops: Animation.Infinite
            running: root.visible
        }
    }
}
