#pragma once

#include <QMainWindow>
#include <QMenuBar>
#include <QSystemTrayIcon>

namespace Ui {
class MainWindow;
}

namespace kemai::app {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event);

private slots:
    void onActionSettingsTriggered();
    void onStackedCurrentChanged(int id);
    void onSystemTrayActivated(QSystemTrayIcon::ActivationReason reason);

private:
    Ui::MainWindow* mUi;

    // keep stacked widgets ids
    int mActivitySId;
    int mSettingsSId;

    // Actions
    QAction* mActQuit     = nullptr;
    QAction* mActSettings = nullptr;

    // Main menu
    QMenuBar* mMenuBar = nullptr;

    // Tray
    QMenu* mTrayMenu                 = nullptr;
    QSystemTrayIcon* mSystemTrayIcon = nullptr;
};

} // namespace kemai::app
