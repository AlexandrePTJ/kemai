// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "pathHelpers.h"

// Qt headers
#include <QDir>
#include <QStandardPaths>

// Project headers
#include "customFmt.h"

namespace
{
    std::optional<QString> gRootPathOverload = std::nullopt;

    QDir getRootDir(kemai::PathHelpers::FilePathType type)
    {
        if (gRootPathOverload.has_value())
        {
            return {gRootPathOverload.value()};
        }

        switch (type)
        {
        case kemai::PathHelpers::FilePathType::Profiles:
        case kemai::PathHelpers::FilePathType::AppSettings:
            return {QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)};

        case kemai::PathHelpers::FilePathType::LogFile:
            return {QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)};

        default:
            throw std::runtime_error("Unknown path type");
        }
    }

    QString getFileName(kemai::PathHelpers::FilePathType type)
    {
        switch (type)
        {
        case kemai::PathHelpers::FilePathType::Profiles:
            return "profiles.json";

        case kemai::PathHelpers::FilePathType::AppSettings:
            // Not "settings.json": Kemai 0.11.x keeps its profiles in that file, same directory
            return "appSettings.json";

        case kemai::PathHelpers::FilePathType::LogFile:
            // Not "kemai.log" at the root: Kemai 0.11.x writes its own log files there
            return "logs/kemai.log";

        default:
            throw std::runtime_error("Unknown path type");
        }
    }

} // namespace

namespace kemai
{
    void PathHelpers::overrideDefaultRootPath(const QString &path)
    {
        gRootPathOverload = path;
    }

    QString PathHelpers::getFilePath(FilePathType type)
    {
        auto dir = getRootDir(type);
        return dir.filePath(getFileName(type));
    }

    void PathHelpers::ensureDirectoryExists(FilePathType type)
    {
        ensureDirectoryExists(getFilePath(type));
    }

    void PathHelpers::ensureDirectoryExists(const QString &path)
    {
        QFileInfo pathInfo(path);
        if (pathInfo.exists())
        {
            return;
        }

        auto dir = pathInfo.isDir() ? QDir(pathInfo.absoluteFilePath()) : pathInfo.absoluteDir();
        if (!dir.mkpath("."))
        {
            throw std::runtime_error(fmt::format("Cannot create {}", dir.path()));
        }
    }

} // namespace kemai