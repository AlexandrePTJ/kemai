#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QActionGroup>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QMessageBox>
#include <QTimer>

#include <spdlog/spdlog.h>

#include "kemai_version.h"

#include "activitywidget.h"
#include "settings.h"
#include "settingsdialog.h"
#include "taskwidget.h"

using namespace kemai::app;
using namespace kemai::client;
using namespace kemai::core;
using namespace kemai::updater;

/*
 * Static helpers
 */
static const auto FirstRequestDelayMs                 = 100;
static const auto MinimalKimaiVersionForPluginRequest = QVersionNumber(1, 14, 1);

/*
 * Class impl
 */
MainWindow::MainWindow() : mUi(new Ui::MainWindow)
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

    mActGroupProfiles = new QActionGroup(this);

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

    mProfileMenu = new QMenu(tr("&Profile"), mMenuBar);

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
    mMenuBar->addMenu(mProfileMenu);
    mMenuBar->addMenu(viewMenu);
    mMenuBar->addMenu(helpMenu);
    setMenuBar(mMenuBar);

    /*
     * Setup widgets
     */
    mActivityWidget = new ActivityWidget;
    mUi->stackedWidget->addWidget(mActivityWidget);

    mTaskWidget = new TaskWidget;
    mUi->stackedWidget->addWidget(mTaskWidget);

    updateProfilesMenu();

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
    connect(mActivityWidget, &ActivityWidget::currentActivityChanged, this, &MainWindow::onActivityChanged);
    connect(mActGroupProfiles, &QActionGroup::triggered, this, &MainWindow::onProfilesActionGroupTriggered);

    /*
     * Delay first refresh and update check
     */
    QTimer::singleShot(FirstRequestDelayMs, this, &MainWindow::processAutoConnect);
    QTimer::singleShot(FirstRequestDelayMs, [&]() {
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

void MainWindow::hideEvent(QHideEvent* event)
{
    auto settings = Settings::load();
    if (settings.kemai.minimizeToSystemTray)
    {
        if (event->spontaneous() && isMinimized())
        {
            hide();
            event->ignore();
        }
    }
    settings.kemai.geometry = saveGeometry();
    Settings::save(settings);
}

void MainWindow::createKimaiClient(const Settings::Profile& profile)
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

        mClient->setHost(profile.host);
        mClient->setUsername(profile.username);
        mClient->setToken(profile.token);

        mSession = QSharedPointer<KemaiSession>::create();

        connect(mClient.data(), &KimaiClient::requestError, this, &MainWindow::onClientError);

        // send some request to identify instance
        auto meResult = mClient->requestMeUserInfo();
        connect(meResult, &KimaiApiBaseResult::ready, this, [this, meResult]() {
            mSession->me = meResult->getResult();
            meResult->deleteLater();
        });
        connect(meResult, &KimaiApiBaseResult::error, this, [this, meResult]() {
            onClientError(meResult->errorMessage());
            meResult->deleteLater();
        });

        auto versionResult = mClient->requestKimaiVersion();
        connect(versionResult, &KimaiApiBaseResult::ready, this, [this, versionResult]() {
            mSession->kimaiVersion = versionResult->getResult().kimai;
            // Allow current client instance to get instance version and list of available plugins. Only available from Kimai 1.14.1
            if (mSession->kimaiVersion >= MinimalKimaiVersionForPluginRequest)
            {
                requestPlugins();
            }
            versionResult->deleteLater();
        });
        connect(versionResult, &KimaiApiBaseResult::error, this, [this, versionResult]() {
            onClientError(versionResult->errorMessage());
            versionResult->deleteLater();
        });

        auto timeSheetConfigResult = mClient->requestTimeSheetConfig();
        connect(timeSheetConfigResult, &KimaiApiBaseResult::ready, this, [this, timeSheetConfigResult]() {
            mSession->timeSheetConfig = timeSheetConfigResult->getResult();
            timeSheetConfigResult->deleteLater();
        });
        connect(timeSheetConfigResult, &KimaiApiBaseResult::error, this, [this, timeSheetConfigResult]() {
            onClientError(timeSheetConfigResult->errorMessage());
            timeSheetConfigResult->deleteLater();
        });

        mActivityWidget->setKimaiClient(mClient);
        mActivityWidget->setKemaiSession(mSession);

        if (mTaskWidget != nullptr)
        {
            mTaskWidget->setKemaiSession(mSession);
        }

        // Save profile connection
        settings.kemai.lastConnectedProfile = profile.id;
        Settings::save(settings);
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
        taskPluginEnabled = mSession->isPluginAvailable(ApiPlugin::TaskManagement);
    }
    mActViewTasks->setEnabled(enable && taskPluginEnabled);
}

void MainWindow::updateProfilesMenu()
{
    auto settings = Settings::load();

    // Removes obsoletes profiles
    for (auto action : mActGroupProfiles->actions())
    {
        if (settings.findProfileRef(action->data().toUuid()) == settings.profiles.end())
        {
            mProfileMenu->removeAction(action);
            mActGroupProfiles->removeAction(action);
            action->deleteLater();
        }
    }

    // Adds new profiles
    for (const auto& profile : settings.profiles)
    {
        bool profileExists = false;
        for (auto action : mActGroupProfiles->actions())
        {
            if (action->data().toUuid() == profile.id)
            {
                profileExists = true;
            }
        }

        if (!profileExists)
        {
            auto action = mProfileMenu->addAction(profile.name);
            action->setCheckable(true);
            action->setData(profile.id);
            mActGroupProfiles->addAction(action);
        }
    }
}

void MainWindow::processAutoConnect()
{
    auto settings = Settings::load();
    if (settings.profiles.isEmpty())
    {
        return;
    }

    auto profileIt = settings.findProfileRef(settings.kemai.lastConnectedProfile);
    if (profileIt == settings.profiles.end())
    {
        profileIt = settings.profiles.begin();
    }

    for (auto& action : mActGroupProfiles->actions())
    {
        if (action->data().toUuid() == profileIt->id)
        {
            action->setChecked(true);
        }
    }
    createKimaiClient(*profileIt);
}

void MainWindow::requestPlugins()
{
    auto pluginsResult = mClient->requestPlugins();

    connect(pluginsResult, &KimaiApiBaseResult::ready, this, [this, pluginsResult]() {
        mSession->plugins = pluginsResult->getResult();

        bool haveTaskPlugin = mSession->isPluginAvailable(ApiPlugin::TaskManagement);
        mActViewTasks->setEnabled(haveTaskPlugin);
        if (haveTaskPlugin)
        {
            mTaskWidget->setKimaiClient(mClient);
        }
        pluginsResult->deleteLater();
    });

    connect(pluginsResult, &KimaiApiBaseResult::error, this, [this, pluginsResult]() {
        onClientError(pluginsResult->errorMessage());
        pluginsResult->deleteLater();
    });
}

void MainWindow::onClientError(const QString& errorMsg)
{
    spdlog::error("Client error: {}", errorMsg.toStdString());
}

void MainWindow::onActionSettingsTriggered()
{
    app::SettingsDialog settingsDialog(this);
    settingsDialog.setSettings(core::Settings::load());
    if (settingsDialog.exec() == QDialog::Accepted)
    {
        core::Settings::save(settingsDialog.settings());

        showSelectedView();
        updateProfilesMenu();
    }
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
        QDesktopServices::openUrl(QUrl::fromUserInput(settings.profiles.first().host));
    }
}

void MainWindow::onSystemTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
    case QSystemTrayIcon::Trigger: {
        auto settings = Settings::load();
        if (isVisible() && (settings.kemai.minimizeToSystemTray || settings.kemai.closeToSystemTray))
        {
            hide();
        }
        else
        {
            showNormal();
            if (!isActiveWindow())
            {
                activateWindow();
            }
        }
    }
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

void MainWindow::onActivityChanged(bool started)
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

void MainWindow::onProfilesActionGroupTriggered(QAction* action)
{
    if (action != nullptr)
    {
        if (action->isChecked())
        {
            auto settings  = Settings::load();
            auto profileId = action->data().toUuid();
            auto profile   = settings.findProfileRef(profileId);
            if (profile != settings.profiles.end())
            {
                createKimaiClient(*profile);
            }
        }
    }
}
