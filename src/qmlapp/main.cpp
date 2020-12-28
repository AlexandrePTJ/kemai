#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "kimaiclientbridge.h"
#include "settingsviewbridge.h"
#include "timesheetviewbridge.h"

using namespace kemai::qmlapp;

int main(int argc, char* argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QGuiApplication app(argc, argv);
    app.setApplicationName("Kemai");
    app.setOrganizationName("Kemai");

    /*
     * Create bridge for QML to communicate with Kimai instance
     */
    KimaiClientBridge kimaiClientBridge;
    kimaiClientBridge.reloadClientSettings();

    /*
     * Create bridge for QML to handle settings
     */
    SettingsViewBridge settingsViewBridge;
    QObject::connect(&settingsViewBridge, &SettingsViewBridge::settingsSaved, &kimaiClientBridge, &KimaiClientBridge::reloadClientSettings);

    TimesheetViewBridge timesheetViewBridge;
    timesheetViewBridge.setProjectModel(kimaiClientBridge.projectDataModel());
    timesheetViewBridge.setActivityModel(kimaiClientBridge.activityDataModel());

    /*
     * Create QML application
     */
    QQmlApplicationEngine engine;

    // setup links between C++ and QML
    engine.rootContext()->setContextProperty("kimaiClientBridge", &kimaiClientBridge);
    engine.rootContext()->setContextProperty("settingsViewBridge", &settingsViewBridge);
    engine.rootContext()->setContextProperty("customerDataModel", kimaiClientBridge.customerDataModel());
    engine.rootContext()->setContextProperty("projectDataModel", timesheetViewBridge.projectFilterModel());
    engine.rootContext()->setContextProperty("activityDataModel", timesheetViewBridge.activityFilterModel());

    // load main page
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject* obj, const QUrl& objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
