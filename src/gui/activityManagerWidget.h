#pragma once

#include <optional>

#include <QToolButton>
#include <QWidget>

#include "client/kimaiClient.h"
#include "context/kemaiSession.h"

namespace Ui {
class ActivityManagerWidget;
}

namespace kemai {

class ActivityManagerWidget : public QWidget
{
    Q_OBJECT

public:
    ActivityManagerWidget(QWidget* parent = nullptr);
    ~ActivityManagerWidget() override;

    void setKemaiSession(std::shared_ptr<KemaiSession> kemaiSession);

private:
    void onSessionCurrentTimeSheetsChanged();
    void onAddActivityWidgetButtonClicked();
    void onSessionCacheSynchronizeFinished();

    std::optional<int> findTabIndexForTimeSheet(const TimeSheet& timeSheet) const;
    int findOrCreateTabForTimeSheet(const TimeSheet& timeSheet);
    void updateTimeSheetTab(int index, const TimeSheet& timeSheet);

    Ui::ActivityManagerWidget* mUi;
    QToolButton* mAddActivityWidgetButton = nullptr;

    std::shared_ptr<KemaiSession> mSession;
};

} // namespace kemai
