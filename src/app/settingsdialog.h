#pragma once

#include <QDialog>
#include <QListWidget>

#include <client/kimaiclient.h>
#include <settings.h>

namespace Ui {
class SettingsDialog;
}

namespace kemai::app {

class SettingsDialog : public QDialog
{
public:
    SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog() override;

    void setSettings(const core::Settings& settings);
    core::Settings settings() const;

private:
    void onProfilesListCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* previous);
    void onBtTestClicked();
    void onClientError(const QString& errorMsg);
    void onClientReply(const client::KimaiReply& reply);
    void onProfileFieldValueChanged();
    void onProfileAddButtonClicked();
    void onProfileDelButtonClicked();

    QScopedPointer<Ui::SettingsDialog> mUi;
    QAction* mActToggleTokenVisible = nullptr;

    core::Settings m_settings;

    // for connection testing
    QScopedPointer<client::KimaiClient> mKimaiClient;
};

} // namespace kemai::app
