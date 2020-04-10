#include <QApplication>
#include <QLibraryInfo>
#include <QTranslator>

#include "mainwindow.h"

using namespace kemai::app;

int main(int argc, char* argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);

    QApplication app(argc, argv);
    app.setApplicationName("Kemai");
    app.setOrganizationName("Kemai");

    // Setup Qt and app translations
    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);

    MainWindow w;
    w.show();

    return app.exec();
}
