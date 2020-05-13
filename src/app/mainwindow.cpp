#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "helpers.h"
#include "kemai_version.h"
#include "settings.h"

#include "activitywidget.h"
#include "settingswidget.h"

#include "activitydialog.h"
#include "customerdialog.h"
#include "projectdialog.h"

#include "kemai/kimairequestfactory.h"

#include <QCloseEvent>
#include <QDebug>
#include <QDesktopServices>
#include <QMessageBox>
#include <QTimer>

using namespace kemai::app;
using namespace kemai::client;
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
    mActNewCustomer = new QAction(tr("New customer..."), this);
    mActNewProject  = new QAction(tr("New project..."), this);
    mActNewActivity = new QAction(tr("New activity..."), this);
    mActCheckUpdate = new QAction(tr("Check for updates..."), this);

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

    auto editMenu = new QMenu(tr("&Edit"), mMenuBar);
    editMenu->addAction(mActNewCustomer);
    editMenu->addAction(mActNewProject);
    editMenu->addAction(mActNewActivity);

    auto helpMenu = new QMenu(tr("&Help"), mMenuBar);
    helpMenu->addAction(mActCheckUpdate);
    helpMenu->addSeparator();
    helpMenu->addAction(tr("About Qt"), qApp, &QApplication::aboutQt);

    mMenuBar->addMenu(fileMenu);
    mMenuBar->addMenu(editMenu);
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
    connect(mActNewCustomer, &QAction::triggered, this, &MainWindow::onActionNewCustomerTriggered);
    connect(mActNewProject, &QAction::triggered, this, &MainWindow::onActionNewProjectTriggered);
    connect(mActNewActivity, &QAction::triggered, this, &MainWindow::onActionNewActivityTriggered);
    connect(mActCheckUpdate, &QAction::triggered, this, &MainWindow::onActionCheckUpdateTriggered);
    connect(mSystemTrayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onSystemTrayActivated);
    connect(&mUpdater, &KemaiUpdater::checkFinished, this, &MainWindow::onNewVersionCheckFinished);

    /*
     * Delay first refresh and update check
     */
    QTimer::singleShot(100, activityWidget, &ActivityWidget::refresh);
    QTimer::singleShot(100, [&]() {
        auto ignoreVersion  = QVersionNumber::fromString(Settings::load().ignoredVersion);
        auto currentVersion = QVersionNumber::fromString(KEMAI_VERSION);
        mUpdater.checkAvailableNewVersion(currentVersion >= ignoreVersion ? currentVersion : ignoreVersion, true);
    });

    // Get client
    refreshClient();
}

MainWindow::~MainWindow()
{
    delete mMenuBar;
    delete mUi;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    auto closeToSysTray = Settings::load().closeToSystemTray;
    if (closeToSysTray)
    {
        hide();
        event->ignore();
    }
}

void MainWindow::refreshClient()
{
    mClient = helpers::createClient();
    if (mClient)
    {
        connect(mClient.data(), &KimaiClient::requestError, this, &MainWindow::onClientError);
        connect(mClient.data(), &KimaiClient::replyReceived, this, &MainWindow::onClientReply);
    }

    mActNewCustomer->setEnabled(mClient != nullptr);
    mActNewProject->setEnabled(mClient != nullptr);
    mActNewActivity->setEnabled(mClient != nullptr);
}

void MainWindow::onClientError(const QString& errorMsg)
{
    qDebug() << errorMsg;
}

void MainWindow::onClientReply(const KimaiReply&)
{
    if (auto activityWidget = qobject_cast<ActivityWidget*>(mUi->stackedWidget->currentWidget()))
    {
        activityWidget->refresh();
    }
}

void MainWindow::onActionSettingsTriggered()
{
    mUi->stackedWidget->setCurrentIndex(mSettingsSId);
}

void MainWindow::onActionNewCustomerTriggered()
{
    auto dialog = CustomerDialog(this);
    if (dialog.exec() == QDialog::Accepted)
    {
        const auto& customer = dialog.customer();
        mClient->sendRequest(KimaiRequestFactory::customerAdd(customer));
    }
}

void MainWindow::onActionNewProjectTriggered()
{
    auto dialog = ProjectDialog(this);
    if (dialog.exec() == QDialog::Accepted)
    {
        const auto& project = dialog.project();
        mClient->sendRequest(KimaiRequestFactory::projectAdd(project));
    }
}

void MainWindow::onActionNewActivityTriggered()
{
    auto dialog = ActivityDialog(this);
    if (dialog.exec() == QDialog::Accepted)
    {
        const auto& activity = dialog.activity();
        mClient->sendRequest(KimaiRequestFactory::activityAdd(activity));
    }
}

void MainWindow::onActionCheckUpdateTriggered()
{
    auto currentVersion = QVersionNumber::fromString(KEMAI_VERSION);
    mUpdater.checkAvailableNewVersion(currentVersion, false);
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

        refreshClient();
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
    if (not details.vn.isNull())
    {
        auto res = QMessageBox::information(
            this, tr("New version available"),
            tr("Version %1 is available.\n\n%2").arg(details.vn.toString()).arg(details.description),
            QMessageBox::Open | QMessageBox::Ignore | QMessageBox::Cancel, QMessageBox::Open);

        switch (res)
        {
        case QMessageBox::Open:
            QDesktopServices::openUrl(details.url);
            break;

        case QMessageBox::Ignore: {
            auto settings           = Settings::load();
            settings.ignoredVersion = details.vn.toString();
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
