// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "logFiles.h"

// Qt headers
#include <QDir>
#include <QFile>
#include <QFileInfo>

// 3rd party headers
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/spdlog.h>

namespace kemai
{
    spdlog::sink_ptr LogFiles::makeRotatingSink(const QString &path, std::size_t maxFileBytes, std::size_t maxFiles, QString *error)
    {
        const auto fail = [error](const QString &reason) -> spdlog::sink_ptr
        {
            if (error != nullptr)
            {
                *error = reason;
            }
            return nullptr;
        };

        if (maxFiles == 0)
        {
            return fail("no file allowed");
        }
        if (!QFileInfo(path).absoluteDir().mkpath("."))
        {
            return fail("the folder can't be created");
        }

        try
        {
            // spdlog counts the rotated files besides the current one
            return std::make_shared<spdlog::sinks::rotating_file_sink_mt>(path.toStdString(), maxFileBytes, maxFiles - 1);
        }
        catch (const spdlog::spdlog_ex &)
        {
            // spdlog's message is technical: ask Qt for the system's reason
            QFile file(path);
            return fail(file.open(QIODevice::Append) ? QString("unknown error") : file.errorString().toLower());
        }
    }
} // namespace kemai
