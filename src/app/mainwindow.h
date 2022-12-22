#pragma once

#include <QActionGroup>
#include <QMainWindow>
#include <QMenuBar>
#include <QSharedPointer>
#include <QSystemTrayIcon>

#include "client/kimaiclient.h"
#include "kemaisession.h"
#include "settings.h"
#include "updater/kemaiupdater.h"

namespace Ui {
class MainWindow;
}

namespace kemai::app {

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
    void createKimaiClient(const core::Settings::Profile& profile);
    void showSelectedView();
    void setViewActionsEnabled(bool enable);
    void updateProfilesMenu();
    void processAutoConnect();

    void onClientError(const QString& errorMsg);
    void onClientReply(const client::KimaiReply& reply);
    void onActionSettingsTriggered();
    void onActionCheckUpdateTriggered();
    void onActionOpenHostTriggered();
    void onSystemTrayActivated(QSystemTrayIcon::ActivationReason reason);
    void onNewVersionCheckFinished(const updater::VersionDetails& details);
    void onActivityChanged(bool started);
    void onProfilesActionGroupTriggered(QAction* action);

    Ui::MainWindow* mUi;
    updater::KemaiUpdater mUpdater;
    QSharedPointer<client::KimaiClient> mClient;
    QSharedPointer<core::KemaiSession> mSession;

    // Request result: because scoped in lambda to process response, instance is 'overwritten', so destructor is called.
    std::shared_ptr<client::VersionRequestResult> mVersionResult;

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
    QActionGroup* mActGroupView     = nullptr;
    QActionGroup* mActGroupProfiles = nullptr;

    // Menus
    QMenuBar* mMenuBar  = nullptr;
    QMenu* mProfileMenu = nullptr;

    // Tray
    QMenu* mTrayMenu                 = nullptr;
    QSystemTrayIcon* mSystemTrayIcon = nullptr;
};

} // namespace kemai::app
