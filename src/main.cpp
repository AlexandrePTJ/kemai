#include <QApplication>
#include <QLibraryInfo>
#include <QTranslator>
#include <QTreeView>

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "client/kimaiClient.h"
#include "gui/loginDialog.h"
#include "gui/mainWindow.h"
#include "kemaiConfig.h"
#include "misc/customFmt.h"
#include "misc/helpers.h"
#include "models/loggerTreeModel.h"
#include "settings/settings.h"

using namespace kemai;

static constinit const auto MaxLogFileSize = 1024 * 102 * 5;

void myMessageOutput(QtMsgType type, const QMessageLogContext& /*context*/, const QString& msg)
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

int main(int argc, char* argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
#endif

    qInstallMessageHandler(myMessageOutput);

    QApplication app(argc, argv);
    QApplication::setApplicationName("Kemai");
    QApplication::setOrganizationName("Kemai");
    QApplication::setApplicationVersion(KEMAI_VERSION);

    // Get kemai data directory and log file path
    auto kemaiSettings = Settings::get();

    // Create Qt logger model before spdlog sinks
    auto loggerTreeModel = std::make_shared<LoggerTreeModel>();

    // Init spdlog console and rotating file (3 x 5Mb)
    auto console = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto rotfile = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(helpers::getLogFilePath().toStdString(), MaxLogFileSize, 3);
    auto qtsink  = std::make_shared<LoggerTreeModelSink>(loggerTreeModel);
    std::vector<spdlog::sink_ptr> sinks{console, rotfile, qtsink};

    auto logger = std::make_shared<spdlog::logger>("kemai", sinks.begin(), sinks.end());
    spdlog::register_logger(logger);
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_default_logger(logger);

    logger->info("===== Starting Kemai {} =====", KEMAI_VERSION);

    // Setup Qt and app translations
    QTranslator qtTranslator;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (qtTranslator.load("qt_" + kemaiSettings.kemai.language.name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
#else
    if (qtTranslator.load("qt_" + kemaiSettings.kemai.language.name(), QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
#endif
    {
        QApplication::installTranslator(&qtTranslator);
    }

    QTranslator appTranslator;
    if (appTranslator.load(kemaiSettings.kemai.language, "kemai", "_", ":/l10n"))
    {
        QApplication::installTranslator(&appTranslator);
    }

    // Setup trusted certificates
    KimaiClient::addTrustedCertificates(kemaiSettings.trustedCertificates);

    // Startup
    MainWindow mainWindow;
    mainWindow.restoreGeometry(kemaiSettings.kemai.geometry);
    mainWindow.setLoggerTreeModel(loggerTreeModel);
    mainWindow.show();

    return QApplication::exec();
}
