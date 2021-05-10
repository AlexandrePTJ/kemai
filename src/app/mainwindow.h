#pragma once

#include <QMainWindow>
#include <QMenuBar>
#include <QSystemTrayIcon>

#include "updater/kemaiupdater.h"

namespace Ui {
class MainWindow;
}

namespace kemai::app {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onActionSettingsTriggered();
    void onActionCheckUpdateTriggered();
    void onActionOpenHostTriggered();
    void onStackedCurrentChanged(int id);
    void onSystemTrayActivated(QSystemTrayIcon::ActivationReason reason);
    void onNewVersionCheckFinished(const updater::VersionDetails& details);
    void onActivityChange(bool started);

private:
    Ui::MainWindow* mUi;
    updater::KemaiUpdater mUpdater;

    // keep stacked widgets ids
    int mActivitySId;
    int mTaskSId;
    int mSettingsSId;
    int mCurrentSId = -1;

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
