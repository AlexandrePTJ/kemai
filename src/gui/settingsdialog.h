#pragma once

#include <QDialog>
#include <QListWidget>

#include "client/kimaiclient.h"
#include "settings/settings.h"

namespace Ui {
class SettingsDialog;
}

namespace kemai {

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog() override;

    void setSettings(const Settings& settings);
    Settings settings() const;

private:
    void onProfilesListCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* previous);
    void onBtTestClicked();
    void onProfileFieldValueChanged();
    void onProfileAddButtonClicked();
    void onProfileDelButtonClicked();

    QScopedPointer<Ui::SettingsDialog> mUi;
    QAction* mActToggleTokenVisible = nullptr;

    Settings m_settings;

    // for connection testing
    QScopedPointer<KimaiClient> mKimaiClient;
};

} // namespace kemai
