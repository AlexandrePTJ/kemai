#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "kimaiclientbridge.h"
#include "settingsviewbridge.h"

using namespace kemai::qmlapp;

int main(int argc, char* argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

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

    /*
     * Create QML application
     */
    QQmlApplicationEngine engine;

    // setup links between C++ and QML
    engine.rootContext()->setContextProperty("kimaiClientBridge", &kimaiClientBridge);
    engine.rootContext()->setContextProperty("settingsViewBridge", &settingsViewBridge);
    engine.rootContext()->setContextProperty("customerDataModel", kimaiClientBridge.customerDataModel());

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
