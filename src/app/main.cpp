#include <QApplication>
#include <QDir>
#include <QLibraryInfo>
#include <QStandardPaths>
#include <QTranslator>

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "mainwindow.h"
#include "settings.h"

using namespace kemai::app;
using namespace kemai::core;
using namespace kemai::client;

int main(int argc, char* argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);

    QApplication app(argc, argv);
    app.setApplicationName("Kemai");
    app.setOrganizationName("Kemai");

    // Get kemai data directory and log file path
    auto appDataDir  = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    auto logFilePath = QDir(appDataDir).absoluteFilePath("kemai.log");

    // Init spdlog console and rotating file (3 x 5Mb)
    auto console = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto rotfile =
        std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logFilePath.toStdString(), 1024 * 1024 * 5, 3);
    std::vector<spdlog::sink_ptr> sinks{console, rotfile};

    auto logger = std::make_shared<spdlog::logger>("kemai", sinks.begin(), sinks.end());
    spdlog::register_logger(logger);
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_default_logger(logger);

    // Setup Qt and app translations
    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);

    // Setup trusted certificates
    auto kemaiSettings = Settings::load();
    KimaiClient::addTrustedCertificates(kemaiSettings.kimai.trustedCertificates);

    // Startup
    MainWindow w;
    w.restoreGeometry(kemaiSettings.kemai.geometry);
    w.show();

    return app.exec();
}
