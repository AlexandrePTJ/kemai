// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <QCommandLineParser>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QLoggingCategory>
#include <QQmlApplicationEngine>
#include <QScopeGuard>

#include "context/logJournal.h"
#include "kemaiConfig.h"
#include "misc/customFmt.h"
#include "misc/logFiles.h"
#include "misc/logJournalSink.h"
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

        // Qt's Wayland text input warns at every focus change out of a text field ("Trying to disable … but 0x0 is
        // focused"): harmless and floods the log. QT_LOGGING_RULES still overrides this for debugging.
        QLoggingCategory::setFilterRules(QStringLiteral("qt.qpa.wayland.textinput.warning=false"));

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
            {{"r", "root"}, "Override default OS configuration/data directories root path", "root"},
            {"verbose", "Also print debug messages on the console"}
        });

        parser.process(app);

        // Before the logger: the log file lives under the root
        if (parser.isSet("root"))
        {
            kemai::PathHelpers::overrideDefaultRootPath(parser.value("root"));
        }

        // Logger setup: console, files and the in-app journal
        kemai::LogJournal logJournal;
        auto              journalSink = std::make_shared<kemai::LogJournalSink>(logJournal.model());
        // The logger outlives the journal
        const auto detachJournal = qScopeGuard([&journalSink]
                                               { journalSink->detach(); });

        std::vector<spdlog::sink_ptr> sinks;
        // The console ends up in the system journal when a session manager starts Kemai: keep debug messages (a request
        // trace every 30 s) out of it unless asked. The files and the in-app journal still get them
        auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        consoleSink->set_level(parser.isSet("verbose") ? spdlog::level::debug : spdlog::level::info);
        sinks.emplace_back(std::move(consoleSink));
        sinks.emplace_back(journalSink);

        const auto logFilePath = kemai::PathHelpers::getFilePath(kemai::PathHelpers::FilePathType::LogFile);
        QString    logFileError;
        if (auto fileSink = kemai::LogFiles::makeRotatingSink(logFilePath, kemai::LogFiles::DefaultMaxFileBytes, kemai::LogFiles::DefaultMaxFiles, &logFileError);
            fileSink != nullptr)
        {
            sinks.emplace_back(std::move(fileSink));
        }
        logJournal.setLogFile(logFilePath, logFileError);

        auto logger = std::make_shared<spdlog::logger>("kemai", sinks.begin(), sinks.end());
        spdlog::register_logger(logger);
        spdlog::set_level(spdlog::level::debug);
        spdlog::set_default_logger(logger);

        spdlog::info("===== Starting Kemai {} ({}) =====", KEMAI_VERSION, KEMAI_CHANNEL);
        if (!logFileError.isEmpty())
        {
            spdlog::error("Cannot write log file {} ({}): logging to the console only", logFilePath, logFileError);
        }

        if (parser.isSet("root"))
        {
            spdlog::warn("Override default OS configuration and data directories to {}", parser.value("root"));
        }

        // Load fonts
        if (const auto fontId = QFontDatabase::addApplicationFont(":/assets/fonts/RaveoVF.ttf"); fontId == -1)
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
        engine.setInitialProperties({
            {"logJournal", QVariant::fromValue(&logJournal)}
        });
        engine.loadFromModule("Kemai", "Main");

        return QGuiApplication::exec();
    }
    catch (std::exception &ex)
    {
        spdlog::critical(ex.what());
    }
    return -1;
}
