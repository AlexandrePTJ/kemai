#pragma once

#include "client/kimaiclient.h"

#include <QWidget>

namespace Ui {
class SettingsWidget;
}

namespace kemai::app {

class SettingsWidget : public QWidget
{
    Q_OBJECT

public:
    SettingsWidget(QWidget* parent = nullptr);
    ~SettingsWidget();

    void setActivityWidgetIndex(int idx);

private slots:
    void backToActivity();
    void loadSettings();
    void saveSettings();

    void onClientError(const QString& errorMsg);
    void onClientReply(const client::KimaiReply& reply);

    void onBtTestClicked();
    void onBtCancelClicked();
    void onBtSaveClicked();

private:
    int mActivityWidgetIndex = 0;
    Ui::SettingsWidget* mUi;

    // for connection testing
    QScopedPointer<client::KimaiClient> mKimaiClient;
};

} // namespace kemai::app
