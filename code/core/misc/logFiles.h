// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// STL headers
#include <cstddef>

// Qt headers
#include <QString>

// 3rd party headers
#include <spdlog/common.h>

namespace kemai
{
    struct LogFiles
    {
        static constexpr std::size_t DefaultMaxFileBytes = 5 * 1024 * 1024;
        static constexpr std::size_t DefaultMaxFiles     = 3;

        // Rotating sink writing `path`, keeping at most `maxFiles` files of `maxFileBytes` (the oldest dropped first).
        // Null when the file cannot be written: Kemai then logs to the console only, and `error` (when given) tells why.
        static spdlog::sink_ptr makeRotatingSink(const QString &path,
                                                 std::size_t    maxFileBytes = DefaultMaxFileBytes,
                                                 std::size_t    maxFiles     = DefaultMaxFiles,
                                                 QString       *error        = nullptr);
    };
} // namespace kemai
