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
    ~SettingsWidget() override;

signals:
    void settingsSaved();
    void cancelled();

private:
    void loadSettings();
    void saveSettings();

    void onClientError(const QString& errorMsg);
    void onClientReply(const client::KimaiReply& reply);

    void onBtTestClicked();
    void onBtCancelClicked();
    void onBtSaveClicked();

private:
    Ui::SettingsWidget* mUi         = nullptr;
    QAction* mActToggleTokenVisible = nullptr;

    // for connection testing
    QScopedPointer<client::KimaiClient> mKimaiClient;
};

} // namespace kemai::app
