#pragma once

// Qt headers
#include <QString>

namespace kemai
{
    class PathHelpers
    {
    public:
        enum class FilePathType
        {
            Profiles
        };

        /*!
         * Use given path instead of OS recommended paths. Only for dev.
         */
        static void overrideDefaultRootPath(const QString& path);

        /*!
         * Get file path for specific path type
         */
        static QString getFilePath(FilePathType type);

        /*!
         * Helpers methods to ensure parent directory of given file exists
         */
        static void ensureDirectoryExists(FilePathType type);
        static void ensureDirectoryExists(const QString& path);
    };
} // namespace kemai
