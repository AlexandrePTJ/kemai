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
    Q_OBJECT

public:
    SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog() override;

    void setSettings(const core::Settings& settings);
    core::Settings settings() const;

private:
    void onProfilesListCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* previous);
    void onBtTestClicked();
    void onProfileFieldValueChanged();
    void onProfileAddButtonClicked();
    void onProfileDelButtonClicked();

    QScopedPointer<Ui::SettingsDialog> mUi;
    QAction* mActToggleTokenVisible = nullptr;

    core::Settings m_settings;

    // for connection testing
    QScopedPointer<client::KimaiClient> mKimaiClient;
    std::shared_ptr<client::VersionRequestResult> mVersionResult;
};

} // namespace kemai::app
