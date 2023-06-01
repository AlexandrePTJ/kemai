#pragma once

#include <QActionGroup>
#include <QMainWindow>
#include <QMenuBar>

#include <QSystemTrayIcon>

#include "client/kimaiClient.h"
#include "context/kemaiSession.h"
#include "monitor/desktopEventsMonitor.h"
#include "settings/settings.h"
#include "updater/kemaiUpdater.h"

namespace Ui {
class MainWindow;
}

namespace kemai {

class ActivityManagerWidget;
class ActivityWidget;
class TaskWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private:
    void createKemaiSession(const Settings::Profile& profile);
    void showSelectedView();
    void setViewActionsEnabled(bool enable);
    void updateProfilesMenu();
    void processAutoConnect();

    void onCurrentTimeSheetsChanged();
    void onPluginsChanged();
    void onActionSettingsTriggered();
    void onActionCheckUpdateTriggered();
    void onActionOpenHostTriggered();
    void onActionRefreshCacheTriggered();
    void onSystemTrayActivated(QSystemTrayIcon::ActivationReason reason);
    void onNewVersionCheckFinished(const VersionDetails& details);
    void onActivityChanged(bool started);
    void onProfilesActionGroupTriggered(QAction* action);
    void onDesktopIdleDetected();
    void onDesktopLockDetected();

    Ui::MainWindow* mUi;
    KemaiUpdater mUpdater;
    std::shared_ptr<KemaiSession> mSession;
    std::shared_ptr<DesktopEventsMonitor> mDesktopEventsMonitor;

    // Stacked widget (ownership is transferred, don't try to delete them)
    ActivityWidget* mActivityWidget               = nullptr;
    ActivityManagerWidget* mActivityManagerWidget = nullptr;
    TaskWidget* mTaskWidget                       = nullptr;

    // Actions
    QAction* mActQuit                  = nullptr;
    QAction* mActSettings              = nullptr;
    QAction* mActCheckUpdate           = nullptr;
    QAction* mActOpenHost              = nullptr;
    QAction* mActViewActivities        = nullptr;
    QAction* mActViewActivitiesManager = nullptr;
    QAction* mActViewTasks             = nullptr;
    QAction* mActRefreshCache          = nullptr;
    QActionGroup* mActGroupView        = nullptr;
    QActionGroup* mActGroupProfiles    = nullptr;

    // Menus
    QMenuBar* mMenuBar  = nullptr;
    QMenu* mProfileMenu = nullptr;

    // Tray
    QMenu* mTrayMenu                 = nullptr;
    QSystemTrayIcon* mSystemTrayIcon = nullptr;
};

} // namespace kemai
