#include "mainWindow.h"
#include "ui_mainWindow.h"

#include <QActionGroup>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QMessageBox>
#include <QTimer>

#include <spdlog/spdlog.h>

#include "aboutDialog.h"
#include "activityWidget.h"
#include "client/kimaiFeatures.h"
#include "kemaiConfig.h"
#include "settings/settings.h"
#include "settingsDialog.h"
#include "taskWidget.h"

using namespace kemai;

/*
 * Static helpers
 */
static const auto FirstRequestDelayMs = 100;

/*
 * Class impl
 */
MainWindow::MainWindow() : mUi(std::make_unique<Ui::MainWindow>())
{
    mUi->setupUi(this);

    const auto& settings = SettingsHelper::load();

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
    mActRefreshCache   = new QAction(tr("Refresh cache"), this);
    mActAboutKemai     = new QAction(tr("About Kemai"), this);
    mActShowLogWidget  = new QAction(tr("Show logs"), this);
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
    mProfileMenu->addAction(mActRefreshCache);
    mProfileMenu->addSeparator();

    auto viewMenu = new QMenu(tr("&View"), mMenuBar);
    viewMenu->addAction(mActViewActivities);
    viewMenu->addAction(mActViewTasks);

    auto helpMenu = new QMenu(tr("&Help"), mMenuBar);
    helpMenu->addAction(mActOpenHost);
    helpMenu->addSeparator();
#ifdef KEMAI_ENABLE_UPDATE_CHECK
    helpMenu->addAction(mActCheckUpdate);
    helpMenu->addSeparator();
#endif // KEMAI_ENABLE_UPDATE_CHECK
    helpMenu->addAction(mActShowLogWidget);
    helpMenu->addSeparator();
    helpMenu->addAction(tr("About Qt"), qApp, &QApplication::aboutQt);
    helpMenu->addAction(mActAboutKemai);

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

    updateProfilesMenu();

    /*
     * Desktop events monitor
     */
    mDesktopEventsMonitor = DesktopEventsMonitor::create(this);
    if (mDesktopEventsMonitor)
    {
        mDesktopEventsMonitor->initialize(settings.events);
        connect(mDesktopEventsMonitor.get(), &DesktopEventsMonitor::idleDetected, this, &MainWindow::onDesktopIdleDetected);
        connect(mDesktopEventsMonitor.get(), &DesktopEventsMonitor::lockDetected, this, &MainWindow::onDesktopLockDetected);
    }

    /*
     * Status bar
     */
    mStatusInstanceLabel.setText(tr("Not connected"));
    mUi->statusbar->addWidget(&mStatusInstanceLabel);

    /*
     * Connections
     */
    connect(mActSettings, &QAction::triggered, this, &MainWindow::onActionSettingsTriggered);
    connect(mActQuit, &QAction::triggered, qApp, &QCoreApplication::quit);
    connect(mActViewActivities, &QAction::triggered, this, &MainWindow::showSelectedView);
    connect(mActViewTasks, &QAction::triggered, this, &MainWindow::showSelectedView);
    connect(mActCheckUpdate, &QAction::triggered, this, &MainWindow::onActionCheckUpdateTriggered);
    connect(mActOpenHost, &QAction::triggered, this, &MainWindow::onActionOpenHostTriggered);
    connect(mActRefreshCache, &QAction::triggered, this, &MainWindow::onActionRefreshCacheTriggered);
    connect(mSystemTrayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onSystemTrayActivated);
    connect(&mUpdater, &KemaiUpdater::checkFinished, this, &MainWindow::onNewVersionCheckFinished);
    connect(mActivityWidget, &ActivityWidget::currentActivityChanged, this, &MainWindow::onActivityChanged);
    connect(mActGroupProfiles, &QActionGroup::triggered, this, &MainWindow::onProfilesActionGroupTriggered);
    connect(mActAboutKemai, &QAction::triggered, this, &MainWindow::onActionAboutKemaiTriggered);
    connect(mActShowLogWidget, &QAction::triggered, &mLoggerWidget, &QWidget::show);

    /*
     * Delay first refresh and update check
     */
    QTimer::singleShot(FirstRequestDelayMs, this, &MainWindow::processAutoConnect);

#ifdef KEMAI_ENABLE_UPDATE_CHECK
    if (settings.kemai.checkUpdateAtStartup)
    {
        QTimer::singleShot(FirstRequestDelayMs, [&]() {
            auto ignoreVersion  = QVersionNumber::fromString(SettingsHelper::load().kemai.ignoredVersion);
            auto currentVersion = QVersionNumber::fromString(KEMAI_VERSION);
            mUpdater.checkAvailableNewVersion(currentVersion >= ignoreVersion ? currentVersion : ignoreVersion, true);
        });
    }
#endif // KEMAI_ENABLE_UPDATE_CHECK
}

MainWindow::~MainWindow()
{
    delete mMenuBar;
}

void MainWindow::setLoggerTreeModel(const std::shared_ptr<LoggerTreeModel>& model)
{
    mLoggerWidget.setModel(model);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (mLoggerWidget.isVisible())
    {
        mLoggerWidget.close();
    }

    auto settings = SettingsHelper::load();
    if (settings.kemai.closeToSystemTray)
    {
        hide();
        event->ignore();
    }
    settings.kemai.geometry = saveGeometry();
    SettingsHelper::save(settings);
}

void MainWindow::hideEvent(QHideEvent* event)
{
    auto settings = SettingsHelper::load();
    if (settings.kemai.minimizeToSystemTray)
    {
        if (event->spontaneous() && isMinimized())
        {
            hide();
            event->ignore();
        }
    }
    settings.kemai.geometry = saveGeometry();
    SettingsHelper::save(settings);
}

void MainWindow::createKemaiSession(const Settings::Profile& profile)
{
    // Clear previous session
    if (mSession)
    {
        mActivityWidget->setKemaiSession(nullptr);
        if (mTaskWidget != nullptr)
        {
            mUi->stackedWidget->removeWidget(mTaskWidget);

            mTaskWidget->deleteLater();
            mTaskWidget = nullptr;
        }
        mStatusInstanceLabel.setText(tr("Not connected"));
    }

    // Ensure profile has all required infos
    const auto haveHost           = !profile.host.isEmpty();
    const auto haveLegacyAuth     = !profile.username.isEmpty() && !profile.token.isEmpty();
    const auto haveAPIToken       = !profile.apiToken.isEmpty();
    const auto isProfileValidated = haveHost && (haveLegacyAuth || haveAPIToken);
    if (!isProfileValidated)
    {
        spdlog::error("Invalid profile. Check settings.");
        return;
    }

    // Create session
    auto kimaiClient = std::make_shared<KimaiClient>();

    kimaiClient->setHost(profile.host);
    kimaiClient->setLegacyAuth(profile.username, profile.token);
    kimaiClient->setAPIToken(profile.apiToken);

    mSession = std::make_shared<KemaiSession>(kimaiClient);
    connect(mSession.get(), &KemaiSession::currentTimeSheetChanged, this, &MainWindow::onCurrentTimeSheetChanged);
    connect(mSession.get(), &KemaiSession::pluginsChanged, this, &MainWindow::onPluginsChanged);
    connect(mSession.get(), &KemaiSession::versionChanged, this, &MainWindow::onSessionVersionChanged);

    mActivityWidget->setKemaiSession(mSession);

    mSession->refreshCache();
    mSession->refreshSessionInfos();
    mSession->refreshCurrentTimeSheet();

    // Save profile connection
    auto settings                       = SettingsHelper::load();
    settings.kemai.lastConnectedProfile = profile.id;
    SettingsHelper::save(settings);
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
    if (mSession)
    {
        taskPluginEnabled = mSession->hasPlugin(ApiPlugin::TaskManagement);
    }
    mActViewTasks->setEnabled(enable && taskPluginEnabled);
}

void MainWindow::updateProfilesMenu()
{
    const auto& settings = SettingsHelper::load();

    // Removes previous profiles
    for (auto action : mActGroupProfiles->actions())
    {
        auto it = std::find_if(settings.profiles.begin(), settings.profiles.end(),
                               [profileId = action->data().toUuid()](const auto& profile) { return profile.id == profileId; });
        if (it != settings.profiles.end())
        {
            mProfileMenu->removeAction(action);
            mActGroupProfiles->removeAction(action);
            action->deleteLater();
        }
    }

    // Add / Update profiles
    auto actions = mActGroupProfiles->actions();
    for (const auto& profile : settings.profiles)
    {
        auto profileAction = std::find_if(actions.begin(), actions.end(), [profile](const auto& action) { return action->data().toUuid() == profile.id; });

        if (profileAction != actions.end())
        {
            (*profileAction)->setText(profile.name);
        }
        else
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
    const auto& settings = SettingsHelper::load();
    if (settings.profiles.empty())
    {
        return;
    }

    auto profileIt = std::find_if(settings.profiles.begin(), settings.profiles.end(),
                                  [profileId = settings.kemai.lastConnectedProfile](const auto& profile) { return profile.id == profileId; });
    if (profileIt != settings.profiles.end())
    {
        for (auto& action : mActGroupProfiles->actions())
        {
            if (action->data().toUuid() == profileIt->id)
            {
                action->setChecked(true);
            }
        }

        createKemaiSession(*profileIt);
    }
}

void MainWindow::onCurrentTimeSheetChanged()
{
    if (mSession && mSession->hasCurrentTimeSheet())
    {
        spdlog::debug("Start desktop monitor");
        mDesktopEventsMonitor->start();
    }
    else
    {
        spdlog::debug("Stop desktop monitor");
        mDesktopEventsMonitor->stop();
    }
}

void MainWindow::onPluginsChanged()
{
    bool haveTaskPlugin = mSession->hasPlugin(ApiPlugin::TaskManagement);
    mActViewTasks->setEnabled(haveTaskPlugin);
    if (haveTaskPlugin)
    {
        if (mTaskWidget == nullptr)
        {
            mTaskWidget = new TaskWidget;
            mUi->stackedWidget->addWidget(mTaskWidget);
        }

        mTaskWidget->setKemaiSession(mSession);
    }
}

void MainWindow::onSessionVersionChanged()
{
    if (!mSession->kimaiVersion().isNull())
    {
        QString apiTokenWarning;
        if (KimaiFeatures::shouldUseAPIToken(mSession->kimaiVersion()) && mSession->client()->isUsingLegacyAuth())
        {
            apiTokenWarning = tr(" - API Token is now recommended. Please migrate your credentials.");
        }

        mStatusInstanceLabel.setText(tr("Kimai %1%2").arg(mSession->kimaiVersion().toString(), apiTokenWarning));
    }
}

void MainWindow::onActionSettingsTriggered()
{
    SettingsDialog settingsDialog(mDesktopEventsMonitor, this);
    settingsDialog.setSettings(SettingsHelper::load());
    if (settingsDialog.exec() == QDialog::Accepted)
    {
        SettingsHelper::save(settingsDialog.settings());

        showSelectedView();
        updateProfilesMenu();

        if (mDesktopEventsMonitor)
        {
            mDesktopEventsMonitor->initialize(settingsDialog.settings().events);
        }
    }
}

void MainWindow::onActionCheckUpdateTriggered()
{
    auto currentVersion = QVersionNumber::fromString(KEMAI_VERSION);
    mUpdater.checkAvailableNewVersion(currentVersion, false);
}

void MainWindow::onActionOpenHostTriggered()
{
    if (mSession)
    {
        QDesktopServices::openUrl(QUrl::fromUserInput(mSession->client()->host()));
    }
}

void MainWindow::onActionRefreshCacheTriggered()
{
    if (mSession)
    {
        mSession->refreshCache();
    }
}

void MainWindow::onActionAboutKemaiTriggered()
{
    AboutDialog aboutDialog(this);
    aboutDialog.exec();
}

void MainWindow::onSystemTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
    case QSystemTrayIcon::Trigger: {
        const auto& settings = SettingsHelper::load();
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
            auto settings                 = SettingsHelper::load();
            settings.kemai.ignoredVersion = details.vn.toString();
            SettingsHelper::save(settings);
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
            auto settings  = SettingsHelper::load();
            auto profileId = action->data().toUuid();
            auto profileIt =
                std::find_if(settings.profiles.begin(), settings.profiles.end(), [profileId](const auto& profile) { return profile.id == profileId; });
            if (profileIt != settings.profiles.end())
            {
                createKemaiSession(*profileIt);
            }
        }
    }
}

void MainWindow::onDesktopIdleDetected()
{
    spdlog::info("System is idle since {} minutes. Stop current TimeSheet.", SettingsHelper::load().events.idleDelayMinutes);
    mActivityWidget->stopCurrentTimeSheet();
}

void MainWindow::onDesktopLockDetected()
{
    spdlog::info("System is locked. Stop current TimeSheet.");
    mActivityWidget->stopCurrentTimeSheet();
}
