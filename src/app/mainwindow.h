#pragma once

#include <QMainWindow>
#include <QMenuBar>
#include <QSharedPointer>
#include <QSystemTrayIcon>

#include "client/kimaiclient.h"
#include "kemaisession.h"
#include "updater/kemaiupdater.h"

namespace Ui {
class MainWindow;
}

namespace kemai::app {

class ActivityWidget;
class SettingsWidget;
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
    void createKimaiClient();
    void showSelectedView();
    void setViewActionsEnabled(bool enable);

    void onClientError(const QString& errorMsg);
    void onClientReply(const client::KimaiReply& reply);
    void onActionSettingsTriggered();
    void onActionCheckUpdateTriggered();
    void onActionOpenHostTriggered();
    void onSystemTrayActivated(QSystemTrayIcon::ActivationReason reason);
    void onNewVersionCheckFinished(const updater::VersionDetails& details);
    void onActivityChanged(bool started);

private:
    Ui::MainWindow* mUi;
    updater::KemaiUpdater mUpdater;
    QSharedPointer<client::KimaiClient> mClient;
    QSharedPointer<core::KemaiSession> mSession;

    // Stacked widget (ownership is transferred, don't try to delete them)
    ActivityWidget* mActivityWidget = nullptr;
    SettingsWidget* mSettingsWidget = nullptr;
    TaskWidget* mTaskWidget         = nullptr;

    // Actions
    QAction* mActQuit           = nullptr;
    QAction* mActSettings       = nullptr;
    QAction* mActCheckUpdate    = nullptr;
    QAction* mActOpenHost       = nullptr;
    QAction* mActViewActivities = nullptr;
    QAction* mActViewTasks      = nullptr;
    QActionGroup* mActGroupView = nullptr;

    // Main menu
    QMenuBar* mMenuBar = nullptr;

    // Tray
    QMenu* mTrayMenu                 = nullptr;
    QSystemTrayIcon* mSystemTrayIcon = nullptr;
};

} // namespace kemai::app
