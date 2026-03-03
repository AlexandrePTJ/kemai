// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include <QCommandLineParser>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "kemaiConfig.h"
#include "misc/customFmt.h"
#include "misc/pathHelpers.h"

void kemaiQtMessageOutputHandler(QtMsgType type, const QMessageLogContext & /*context*/, const QString &msg)
{
    switch (type)
    {
    case QtDebugMsg:
        spdlog::debug(msg);
        break;
    case QtInfoMsg:
        spdlog::info(msg);
        break;
    case QtWarningMsg:
        spdlog::warn(msg);
        break;
    case QtCriticalMsg:
    case QtFatalMsg:
        spdlog::critical(msg);
        break;
    }
}

int main(int argc, char *argv[])
{
    try
    {
        // Catch Qt messages to spdlog
        qInstallMessageHandler(kemaiQtMessageOutputHandler);

        // QtApp setup
        QGuiApplication app(argc, argv);
        QGuiApplication::setApplicationName("Kemai");
        QGuiApplication::setOrganizationName("Kemai");
        QGuiApplication::setApplicationVersion(KEMAI_VERSION);

        // CLI options
        QCommandLineParser parser;
        parser.addHelpOption();
        parser.addVersionOption();

        parser.addOptions({
            {{"r", "root"}, "Override default OS configuration/data directories root path", "root"}
        });

        parser.process(app);

        // Logger setup
        std::vector<spdlog::sink_ptr> sinks;
        sinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

        auto logger = std::make_shared<spdlog::logger>("kemai", sinks.begin(), sinks.end());
        spdlog::register_logger(logger);
        spdlog::set_level(spdlog::level::debug);
        spdlog::set_default_logger(logger);

        spdlog::info("===== Starting Kemai {} =====", KEMAI_VERSION);

        // Check CLI options
        if (parser.isSet("root"))
        {
            spdlog::warn("Override default OS configuration and data directories to {}", parser.value("root"));
            kemai::PathHelpers::overrideDefaultRootPath(parser.value("root"));
        }

        // Load fonts
        if (const auto fontId = QFontDatabase::addApplicationFont(":/Kemai/assets/fonts/RaveoVF.ttf"); fontId == -1)
        {
            spdlog::warn("Failed to load font RaveoVF.ttf");
        }
        else
        {
            auto fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
            QGuiApplication::setFont(fontFamilies.first());
        }

        // Setup QML and run
        QQmlApplicationEngine engine;
        engine.loadFromModule("Kemai", "Main");

        return QGuiApplication::exec();
    }
    catch (std::exception &ex)
    {
        spdlog::critical(ex.what());
    }
    return -1;
}
