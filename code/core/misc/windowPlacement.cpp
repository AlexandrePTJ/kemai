// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "windowPlacement.h"

// STL headers
#include <algorithm>

namespace
{
    // Less than this on a screen (e.g. a title bar sliver) is not something the user can grab
    constexpr int MinVisibleWidth  = 100;
    constexpr int MinVisibleHeight = 50;
} // namespace

namespace kemai
{
    WindowPlacement WindowPlacement::fit(const QRect &saved, const QList<QRect> &availableScreens)
    {
        if (availableScreens.isEmpty())
        {
            return {.rect = saved, .positionKept = true};
        }

        // Screen holding most of the window
        const QRect *best   = nullptr;
        qint64       bestArea = 0;
        for (const auto &screen : availableScreens)
        {
            const auto overlap = screen.intersected(saved);
            const auto area    = static_cast<qint64>(overlap.width()) * overlap.height();
            if (area > bestArea)
            {
                best     = &screen;
                bestArea = area;
            }
        }

        const auto visibleEnough = [&saved](const QRect &overlap)
        {
            return overlap.width() >= std::min(MinVisibleWidth, saved.width()) && overlap.height() >= std::min(MinVisibleHeight, saved.height());
        };

        if (best == nullptr || !visibleEnough(best->intersected(saved)))
        {
            const auto &primary = availableScreens.first();
            return {.rect = QRect(primary.topLeft(), saved.size().boundedTo(primary.size())), .positionKept = false};
        }

        const auto size = saved.size().boundedTo(best->size());
        const auto x    = std::clamp(saved.x(), best->left(), best->left() + best->width() - size.width());
        const auto y    = std::clamp(saved.y(), best->top(), best->top() + best->height() - size.height());
        return {.rect = QRect(QPoint(x, y), size), .positionKept = true};
    }
} // namespace kemai
