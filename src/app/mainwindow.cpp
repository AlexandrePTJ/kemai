#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "activitywidget.h"
#include "settings.h"
#include "settingswidget.h"

#include "customerdialog.h"

#include "kemai/kimairequestfactory.h"

#include <QCloseEvent>
#include <QTimer>
#include <QDebug>

using namespace kemai::app;
using namespace kemai::client;

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

    mMenuBar->addMenu(fileMenu);
    mMenuBar->addMenu(editMenu);
    setMenuBar(mMenuBar);

    /*
     * Setup widgets
     */
    auto activityWidget = new ActivityWidget;
    mActivitySId        = mUi->stackedWidget->addWidget(activityWidget);

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
    connect(mSystemTrayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onSystemTrayActivated);

    /*
     * Delay first refresh
     */
    QTimer::singleShot(100, activityWidget, &ActivityWidget::refresh);

    // TODO: Move to factory
    auto settings = Settings::load();
    if (settings.isReady())
    {
        mClient.reset(new KimaiClient);
        mClient->setHost(settings.host);
        mClient->setUsername(settings.username);
        mClient->setToken(settings.token);
        connect(mClient.data(), &KimaiClient::requestError, this, &MainWindow::onClientError);
    }
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

void MainWindow::onClientError(const QString& errorMsg)
{
    qDebug() << errorMsg;
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

void MainWindow::onStackedCurrentChanged(int id)
{
    if (id == mActivitySId)
    {
        if (auto activityWidget = qobject_cast<ActivityWidget*>(mUi->stackedWidget->widget(id)))
        {
            activityWidget->refresh();
        }
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
