// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "logFiles.h"

// Qt headers
#include <QDir>
#include <QFileInfo>

// 3rd party headers
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/spdlog.h>

namespace kemai
{
    spdlog::sink_ptr LogFiles::makeRotatingSink(const QString &path, std::size_t maxFileBytes, std::size_t maxFiles)
    {
        if (maxFiles == 0 || !QFileInfo(path).absoluteDir().mkpath("."))
        {
            return nullptr;
        }

        try
        {
            // spdlog counts the rotated files besides the current one
            return std::make_shared<spdlog::sinks::rotating_file_sink_mt>(path.toStdString(), maxFileBytes, maxFiles - 1);
        }
        catch (const spdlog::spdlog_ex &)
        {
            return nullptr;
        }
    }
} // namespace kemai
