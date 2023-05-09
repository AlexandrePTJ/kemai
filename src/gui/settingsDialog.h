#pragma once

#include <memory>

#include <QDialog>
#include <QListWidget>

#include "client/kimaiClient.h"
#include "monitor/desktopEventsMonitor.h"
#include "settings/settings.h"

namespace Ui {
class SettingsDialog;
}

namespace kemai {

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    SettingsDialog(const std::shared_ptr<DesktopEventsMonitor>& desktopEventsMonitor, QWidget* parent = nullptr);
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
    std::unique_ptr<KimaiClient> mKimaiClient;
};

} // namespace kemai
