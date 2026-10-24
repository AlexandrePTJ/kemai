// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include <catch2/catch_test_macros.hpp>

#include <misc/windowPlacement.h>

using namespace kemai;

namespace
{
    // Primary 1920x1050 available, second monitor on its right
    const QList<QRect> TwoScreens{QRect(0, 0, 1920, 1050), QRect(1920, 0, 2560, 1400)};
} // namespace

TEST_CASE("WindowPlacement::fit - fully visible window is unchanged", "[windowPlacement][unit]")
{
    const QRect saved(2200, 100, 900, 700);
    const auto  placed = WindowPlacement::fit(saved, TwoScreens);

    REQUIRE(placed.positionKept);
    REQUIRE(placed.rect == saved);
}

TEST_CASE("WindowPlacement::fit - window off every screen keeps only its size", "[windowPlacement][unit]")
{
    // Was on the second monitor, now unplugged
    const auto placed = WindowPlacement::fit(QRect(2200, 100, 900, 700), {QRect(0, 0, 1920, 1050)});

    REQUIRE_FALSE(placed.positionKept);
    REQUIRE(placed.rect.size() == QSize(900, 700));
}

TEST_CASE("WindowPlacement::fit - window larger than the screen is shrunk to it", "[windowPlacement][unit]")
{
    const QRect screen(0, 0, 1366, 728);
    const auto  placed = WindowPlacement::fit(QRect(100, 50, 1600, 1000), {screen});

    REQUIRE(placed.positionKept);
    REQUIRE(screen.contains(placed.rect));
    REQUIRE(placed.rect.size() == QSize(1366, 728));

    const auto offScreen = WindowPlacement::fit(QRect(5000, 5000, 1600, 1000), {screen});
    REQUIRE_FALSE(offScreen.positionKept);
    REQUIRE(offScreen.rect.size() == QSize(1366, 728));
}

TEST_CASE("WindowPlacement::fit - partially visible window is moved fully onto its screen", "[windowPlacement][unit]")
{
    // Hanging over the bottom-right corner of the primary screen only
    const auto placed = WindowPlacement::fit(QRect(1700, 900, 800, 600), {QRect(0, 0, 1920, 1050)});

    REQUIRE(placed.positionKept);
    REQUIRE(placed.rect == QRect(1120, 450, 800, 600));
}

TEST_CASE("WindowPlacement::fit - a sliver on screen does not count as visible", "[windowPlacement][unit]")
{
    // Only 20 px of the window's left edge on the screen
    const auto placed = WindowPlacement::fit(QRect(1900, 100, 800, 600), {QRect(0, 0, 1920, 1050)});

    REQUIRE_FALSE(placed.positionKept);
}
