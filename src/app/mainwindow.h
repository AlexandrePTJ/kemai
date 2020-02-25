#pragma once

#include <QMainWindow>

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

private slots:
    void onActionSettingsTriggered();
    void onStackedCurrentChanged(int id);

private:
    Ui::MainWindow* mUi;

    // keep stacked widgets ids
    int mActivitySId;
    int mSettingsSId;
};

} // namespace kemai::app
