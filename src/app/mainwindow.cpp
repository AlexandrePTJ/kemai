#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCloseEvent>
#include <QDesktopServices>
#include <QMessageBox>
#include <QTimer>

#include <spdlog/spdlog.h>

#include "client/kimairequestfactory.h"
#include "kemai_version.h"

#include "activitywidget.h"
#include "settings.h"
#include "settingswidget.h"
#include "taskwidget.h"

using namespace kemai::app;
using namespace kemai::client;
using namespace kemai::core;
using namespace kemai::updater;

MainWindow::MainWindow() : QMainWindow(), mUi(new Ui::MainWindow)
{
    mUi->setupUi(this);

    /*
     * Setup icon
     */
    QIcon icon(":/icons/kemai");
    setWindowIcon(icon);

    /*
     * Setup actions
     */
    mActQuit           = new QAction(tr("&Quit"), this);
    mActSettings       = new QAction(tr("&Settings"), this);
    mActCheckUpdate    = new QAction(tr("Check for updates..."), this);
    mActOpenHost       = new QAction(tr("Open Kimai instance"), this);
    mActViewActivities = new QAction(tr("Activities"), this);
    mActViewTasks      = new QAction(tr("Tasks"), this);
    mActViewTasks->setEnabled(false);

    mActViewActivities->setCheckable(true);
    mActViewTasks->setCheckable(true);
    mActViewActivities->setChecked(true);

    mActGroupView = new QActionGroup(this);
    mActGroupView->addAction(mActViewActivities);
    mActGroupView->addAction(mActViewTasks);

    /*
     * Setup system tray
     */
    mTrayMenu = new QMenu(this);
    mTrayMenu->addAction(mActSettings);
    mTrayMenu->addSeparator();
    mTrayMenu->addAction(mActQuit);

    mSystemTrayIcon = new QSystemTrayIcon(this);
    mSystemTrayIcon->setContextMenu(mTrayMenu);
    mSystemTrayIcon->setIcon(icon);
    mSystemTrayIcon->show();

    /*
     * Setup main menu
     */
    mMenuBar = new QMenuBar;

    auto fileMenu = new QMenu(tr("&File"), mMenuBar);
    fileMenu->addAction(mActSettings);
    fileMenu->addSeparator();
    fileMenu->addAction(mActQuit);

    auto viewMenu = new QMenu(tr("&View"), mMenuBar);
    viewMenu->addAction(mActViewActivities);
    viewMenu->addAction(mActViewTasks);

    auto helpMenu = new QMenu(tr("&Help"), mMenuBar);
    helpMenu->addAction(mActOpenHost);
    helpMenu->addSeparator();
    helpMenu->addAction(mActCheckUpdate);
    helpMenu->addSeparator();
    helpMenu->addAction(tr("About Qt"), qApp, &QApplication::aboutQt);

    mMenuBar->addMenu(fileMenu);
    mMenuBar->addMenu(viewMenu);
    mMenuBar->addMenu(helpMenu);
    setMenuBar(mMenuBar);

    /*
     * Setup widgets
     */
    mActivityWidget = new ActivityWidget;
    mUi->stackedWidget->addWidget(mActivityWidget);

    mSettingsWidget = new SettingsWidget;
    mUi->stackedWidget->addWidget(mSettingsWidget);

    mTaskWidget = new TaskWidget;
    mUi->stackedWidget->addWidget(mTaskWidget);

    /*
     * Connections
     */
    connect(mActSettings, &QAction::triggered, this, &MainWindow::onActionSettingsTriggered);
    connect(mActQuit, &QAction::triggered, qApp, &QCoreApplication::quit);
    connect(mActViewActivities, &QAction::triggered, this, &MainWindow::showSelectedView);
    connect(mActViewTasks, &QAction::triggered, this, &MainWindow::showSelectedView);
    connect(mActCheckUpdate, &QAction::triggered, this, &MainWindow::onActionCheckUpdateTriggered);
    connect(mActOpenHost, &QAction::triggered, this, &MainWindow::onActionOpenHostTriggered);
    connect(mSystemTrayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onSystemTrayActivated);
    connect(&mUpdater, &KemaiUpdater::checkFinished, this, &MainWindow::onNewVersionCheckFinished);
    connect(mActivityWidget, &ActivityWidget::currentActivityChanged, this, &MainWindow::onActivityChange);
    connect(mSettingsWidget, &SettingsWidget::cancelled, this, &MainWindow::showSelectedView);
    connect(mSettingsWidget, &SettingsWidget::settingsSaved, [&]() {
        createKimaiClient();
        showSelectedView();
    });

    /*
     * Delay first refresh and update check
     */
    QTimer::singleShot(100, this, &MainWindow::createKimaiClient);
    QTimer::singleShot(100, [&]() {
        auto ignoreVersion  = QVersionNumber::fromString(Settings::load().kemai.ignoredVersion);
        auto currentVersion = QVersionNumber::fromString(KEMAI_VERSION);
        mUpdater.checkAvailableNewVersion(currentVersion >= ignoreVersion ? currentVersion : ignoreVersion, true);
    });
}

MainWindow::~MainWindow()
{
    delete mMenuBar;
    delete mUi;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    auto settings = Settings::load();
    if (settings.kemai.closeToSystemTray)
    {
        hide();
        event->ignore();
    }
    settings.kemai.geometry = saveGeometry();
    Settings::save(settings);
}

void MainWindow::createKimaiClient()
{
    // Clear previous client usage
    if (mClient)
    {
        mActivityWidget->setKimaiClient(nullptr);
    }

    auto settings = Settings::load();
    if (settings.isReady())
    {
        mClient = QSharedPointer<KimaiClient>::create();
        mClient->setHost(settings.kimai.host);
        mClient->setUsername(settings.kimai.username);
        mClient->setToken(settings.kimai.token);

        connect(mClient.data(), &KimaiClient::replyReceived, this, &MainWindow::onClientReply);
        connect(mClient.data(), &KimaiClient::requestError, this, &MainWindow::onClientError);

        // send some request to identify instance
        mClient->sendRequest(KimaiRequestFactory::version());

        mActivityWidget->setKimaiClient(mClient);
    }
}

void MainWindow::showSelectedView()
{
    setViewActionsEnabled(true);

    if (mActViewTasks->isChecked())
    {
        mUi->stackedWidget->setCurrentWidget(mTaskWidget);
    }
    else
    {
        mUi->stackedWidget->setCurrentWidget(mActivityWidget);
    }
}

void MainWindow::setViewActionsEnabled(bool enable)
{
    mActViewActivities->setEnabled(enable);

    bool taskPluginEnabled = false;
    if (mClient)
    {
        taskPluginEnabled = mClient->isPluginAvailable(ApiPlugin::TaskManagement);
    }
    mActViewTasks->setEnabled(enable && taskPluginEnabled);
}

void MainWindow::onClientError(const QString& errorMsg)
{
    spdlog::error("Client error: {}", errorMsg.toStdString());
}

void MainWindow::onClientReply(const KimaiReply& reply)
{
    if (!reply.isValid())
        return;

    switch (reply.method())
    {
    case ApiMethod::Version: {
        // Allow current client instance to get instance version and list of available plugins.
        mClient->sendRequest(KimaiRequestFactory::plugins());
    }
    break;

    case ApiMethod::Plugins: {
        mActViewTasks->setEnabled(mClient->isPluginAvailable(ApiPlugin::TaskManagement));
        if (mActViewTasks->isEnabled())
        {
            mTaskWidget->setKimaiClient(mClient);
        }
    }
    break;

    default:
        break;
    }
}

void MainWindow::onActionSettingsTriggered()
{
    setViewActionsEnabled(false);
    mUi->stackedWidget->setCurrentWidget(mSettingsWidget);
}

void MainWindow::onActionCheckUpdateTriggered()
{
    auto currentVersion = QVersionNumber::fromString(KEMAI_VERSION);
    mUpdater.checkAvailableNewVersion(currentVersion, false);
}

void MainWindow::onActionOpenHostTriggered()
{
    auto settings = Settings::load();
    if (settings.isReady())
    {
        QDesktopServices::openUrl(QUrl::fromUserInput(settings.kimai.host));
    }
}

void MainWindow::onSystemTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
    case QSystemTrayIcon::Trigger:
        showNormal();
        break;

    default:
        break;
    }
}

void MainWindow::onNewVersionCheckFinished(const VersionDetails& details)
{
    if (!details.vn.isNull())
    {
        auto res =
            QMessageBox::information(this, tr("New version available"), tr("Version %1 is available.\n\n%2").arg(details.vn.toString(), details.description),
                                     QMessageBox::Open | QMessageBox::Ignore | QMessageBox::Cancel, QMessageBox::Open);

        switch (res)
        {
        case QMessageBox::Open:
            QDesktopServices::openUrl(details.url);
            break;

        case QMessageBox::Ignore: {
            auto settings                 = Settings::load();
            settings.kemai.ignoredVersion = details.vn.toString();
            Settings::save(settings);
        }
        break;

        default:
            break;
        }
    }
    else
    {
        QMessageBox::information(this, tr("No update"), tr("%1 is latest version.").arg(KEMAI_VERSION));
    }
}

void MainWindow::onActivityChange(bool started)
{
    if (started)
    {
        setWindowIcon(QIcon(":/icons/kemai"));
        mSystemTrayIcon->setIcon(QIcon(":/icons/kemai"));
    }
    else
    {
        setWindowIcon(QIcon(":/icons/kemai-red"));
        mSystemTrayIcon->setIcon(QIcon(":/icons/kemai-red"));
    }
}
