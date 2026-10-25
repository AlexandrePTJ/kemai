// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// Qt headers
#include <QList>
#include <QRect>

namespace kemai
{
    struct WindowPlacement
    {
        QRect rect;
        // false: the saved position was off every screen, only rect's size applies (the platform places the window)
        bool positionKept = false;

        // Fits a saved window geometry to the available geometries of the connected screens (primary first):
        // the position is kept when enough of the window is on a screen, then the window is moved and shrunk
        // to lie fully on it; otherwise only the size is kept, shrunk to the primary screen.
        static WindowPlacement fit(const QRect &saved, const QList<QRect> &availableScreens);
    };
} // namespace kemai
