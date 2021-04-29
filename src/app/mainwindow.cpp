#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "kemai_version.h"

#include "activitywidget.h"
#include "settingswidget.h"

#include "client/kimairequestfactory.h"
#include "settings.h"

#include <QCloseEvent>
#include <QDesktopServices>
#include <QMessageBox>
#include <QTimer>

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
    mActQuit        = new QAction(tr("&Quit"), this);
    mActSettings    = new QAction(tr("&Settings"), this);
    mActCheckUpdate = new QAction(tr("Check for updates..."), this);
    mActOpenHost    = new QAction(tr("Open Kimai instance"), this);

    /*
     * Setup systemtray
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

    auto helpMenu = new QMenu(tr("&Help"), mMenuBar);
    helpMenu->addAction(mActOpenHost);
    helpMenu->addSeparator();
    helpMenu->addAction(mActCheckUpdate);
    helpMenu->addSeparator();
    helpMenu->addAction(tr("About Qt"), qApp, &QApplication::aboutQt);

    mMenuBar->addMenu(fileMenu);
    mMenuBar->addMenu(helpMenu);
    setMenuBar(mMenuBar);

    /*
     * Setup widgets
     */
    auto activityWidget = new ActivityWidget;
    mActivitySId        = mUi->stackedWidget->addWidget(activityWidget);
    mCurrentSId         = mActivitySId;

    auto settingsWidget = new SettingsWidget;
    settingsWidget->setActivityWidgetIndex(mActivitySId);
    mSettingsSId = mUi->stackedWidget->addWidget(settingsWidget);

    /*
     * Connections
     */
    connect(mUi->stackedWidget, &QStackedWidget::currentChanged, this, &MainWindow::onStackedCurrentChanged);
    connect(mActSettings, &QAction::triggered, this, &MainWindow::onActionSettingsTriggered);
    connect(mActQuit, &QAction::triggered, qApp, &QCoreApplication::quit);
    connect(mActCheckUpdate, &QAction::triggered, this, &MainWindow::onActionCheckUpdateTriggered);
    connect(mActOpenHost, &QAction::triggered, this, &MainWindow::onActionOpenHostTriggered);
    connect(mSystemTrayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onSystemTrayActivated);
    connect(&mUpdater, &KemaiUpdater::checkFinished, this, &MainWindow::onNewVersionCheckFinished);
    connect(activityWidget, &ActivityWidget::currentActivityChanged, this, &MainWindow::onActivityChange);

    /*
     * Delay first refresh and update check
     */
    QTimer::singleShot(100, activityWidget, &ActivityWidget::refresh);
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

void MainWindow::onActionSettingsTriggered()
{
    mUi->stackedWidget->setCurrentIndex(mSettingsSId);
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

void MainWindow::onStackedCurrentChanged(int id)
{
    // Check if we left settings stack
    if (mCurrentSId == mSettingsSId)
    {
        if (id == mActivitySId)
        {
            if (auto activityWidget = qobject_cast<ActivityWidget*>(mUi->stackedWidget->widget(id)))
            {
                activityWidget->refresh();
            }
        }
    }

    mCurrentSId = id;
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
        auto res = QMessageBox::information(
            this, tr("New version available"),
            tr("Version %1 is available.\n\n%2").arg(details.vn.toString(), details.description),
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
