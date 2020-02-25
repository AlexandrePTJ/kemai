#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "activitywidget.h"
#include "settingswidget.h"

#include <QTimer>

using namespace kemai::app;

MainWindow::MainWindow() : QMainWindow(), mUi(new Ui::MainWindow)
{
    mUi->setupUi(this);

    auto activityWidget = new ActivityWidget;
    mActivitySId        = mUi->stackedWidget->addWidget(activityWidget);

    auto settingsWidget = new SettingsWidget;
    settingsWidget->setActivityWidgetIndex(mActivitySId);
    mSettingsSId = mUi->stackedWidget->addWidget(settingsWidget);

    connect(mUi->actionSettings, &QAction::triggered, this, &MainWindow::onActionSettingsTriggered);
    connect(mUi->stackedWidget, &QStackedWidget::currentChanged, this, &MainWindow::onStackedCurrentChanged);

    QTimer::singleShot(500, activityWidget, &ActivityWidget::refresh);
}

MainWindow::~MainWindow()
{
    delete mUi;
}

void MainWindow::onActionSettingsTriggered()
{
    mUi->stackedWidget->setCurrentIndex(mSettingsSId);
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
