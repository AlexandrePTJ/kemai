#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "settingsviewbridge.h"

using namespace kemai::qmlapp;

int main(int argc, char* argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);
    app.setApplicationName("Kemai");
    app.setOrganizationName("Kemai");

    SettingsViewBridge settingsViewBridge;

    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("settingsViewBridge", &settingsViewBridge);

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
