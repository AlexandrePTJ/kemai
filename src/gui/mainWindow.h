#pragma once

#include <QActionGroup>
#include <QLabel>
#include <QMainWindow>
#include <QMenuBar>
#include <QNetworkAccessManager>
#include <QSystemTrayIcon>

#include "client/kimaiClient.h"
#include "context/kemaiSession.h"
#include "gui/loggerWidget.h"
#include "monitor/desktopEventsMonitor.h"
#include "settings/settings.h"
#include "updater/kemaiUpdater.h"

namespace Ui {
class MainWindow;
}

namespace kemai {

class ActivityWidget;
class TaskWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow() override;

    void setLoggerTreeModel(const std::shared_ptr<LoggerTreeModel>& model);

protected:
    void closeEvent(QCloseEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private:
    void createKemaiSession(const Settings::Profile& profile);
    void showSelectedView();
    void setViewActionsEnabled(bool enable);
    void updateProfilesMenu();
    void processAutoConnect();

    void onNamSslErrors(QNetworkReply* reply, const QList<QSslError>& errors);
    void onCurrentTimeSheetChanged();
    void onPluginsChanged();
    void onSessionVersionChanged();
    void onActionSettingsTriggered();
    void onActionCheckUpdateTriggered();
    void onActionOpenHostTriggered();
    void onActionRefreshCacheTriggered();
    void onActionAboutKemaiTriggered();
    void onSystemTrayActivated(QSystemTrayIcon::ActivationReason reason);
    void onNewVersionCheckFinished(const VersionDetails& details);
    void onActivityChanged(bool started);
    void onProfilesActionGroupTriggered(QAction* action);
    void onDesktopIdleDetected();
    void onDesktopLockDetected();

    std::unique_ptr<Ui::MainWindow> mUi;
    KemaiUpdater mUpdater;
    std::shared_ptr<KemaiSession> mSession;
    std::shared_ptr<DesktopEventsMonitor> mDesktopEventsMonitor;
    std::shared_ptr<QNetworkAccessManager> mNam;

    // Stacked widget (ownership is transferred, don't try to delete them)
    ActivityWidget* mActivityWidget = nullptr;
    TaskWidget* mTaskWidget         = nullptr;

    // Actions
    QAction* mActQuit               = nullptr;
    QAction* mActSettings           = nullptr;
    QAction* mActCheckUpdate        = nullptr;
    QAction* mActOpenHost           = nullptr;
    QAction* mActViewActivities     = nullptr;
    QAction* mActViewTasks          = nullptr;
    QAction* mActRefreshCache       = nullptr;
    QAction* mActAboutKemai         = nullptr;
    QAction* mActShowLogWidget      = nullptr;
    QActionGroup* mActGroupView     = nullptr;
    QActionGroup* mActGroupProfiles = nullptr;

    // Menus
    QMenuBar* mMenuBar  = nullptr;
    QMenu* mProfileMenu = nullptr;

    // Tray
    QMenu* mTrayMenu                 = nullptr;
    QSystemTrayIcon* mSystemTrayIcon = nullptr;

    // Status bar
    QLabel mStatusInstanceLabel;

    // Logger view
    LoggerWidget mLoggerWidget;
};

} // namespace kemai
