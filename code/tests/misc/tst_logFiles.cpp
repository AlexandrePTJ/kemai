// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include <catch2/catch_test_macros.hpp>

#include <QDir>
#include <QFileInfo>
#include <QTemporaryDir>

#include <spdlog/spdlog.h>

#include <misc/logFiles.h>
#include <misc/pathHelpers.h>

using namespace kemai;

TEST_CASE("LogFiles - log file is apart from Kemai 0.11's kemai.log", "[logFiles][unit]")
{
    QTemporaryDir tempDir;
    REQUIRE(tempDir.isValid());
    PathHelpers::overrideDefaultRootPath(tempDir.path());

    const auto path = PathHelpers::getFilePath(PathHelpers::FilePathType::LogFile);
    REQUIRE(QFileInfo(path).fileName() == "kemai.log");
    REQUIRE(QFileInfo(path).absolutePath() == QDir(tempDir.path()).filePath("logs"));
}

TEST_CASE("LogFiles::makeRotatingSink - creates the directory and rotates", "[logFiles][unit]")
{
    QTemporaryDir tempDir;
    REQUIRE(tempDir.isValid());
    const auto path = QDir(tempDir.path()).filePath("logs/kemai.log");

    {
        auto sink = LogFiles::makeRotatingSink(path, 1024, 3);
        REQUIRE(sink != nullptr);

        spdlog::logger logger("rotation-test", sink);
        for (int i = 0; i < 200; ++i)
        {
            logger.info("line {} of a log long enough to fill several kilobytes", i);
        }
        logger.flush();
    }

    const auto files = QDir(QFileInfo(path).absolutePath()).entryInfoList(QDir::Files);
    REQUIRE(files.size() == 3);
    for (const auto &file : files)
    {
        REQUIRE(file.size() <= 1024);
    }
}

TEST_CASE("LogFiles::makeRotatingSink - unwritable directory gives no sink", "[logFiles][unit]")
{
    QTemporaryDir tempDir;
    REQUIRE(tempDir.isValid());

    // A file where the log directory should be
    QFile blocker(QDir(tempDir.path()).filePath("logs"));
    REQUIRE(blocker.open(QIODevice::WriteOnly));
    blocker.close();

    REQUIRE(LogFiles::makeRotatingSink(QDir(tempDir.path()).filePath("logs/kemai.log")) == nullptr);
}
