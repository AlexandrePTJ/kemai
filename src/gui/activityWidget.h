#pragma once

#include <QTimer>
#include <QWidget>

#include "client/kimaiClient.h"
#include "context/kemaiSession.h"

namespace Ui {
class ActivityWidget;
}

namespace kemai {

class ActivityWidget : public QWidget
{
    Q_OBJECT

public:
    ActivityWidget(QWidget* parent = nullptr);
    ~ActivityWidget() override;

    void setKemaiSession(std::shared_ptr<KemaiSession> kemaiSession);
    void stopCurrentTimeSheet();

signals:
    void currentActivityChanged(bool started);

private:
    void onCbCustomerFieldChanged();
    void onCbProjectFieldChanged();
    void onCbActivityFieldChanged();

    void onTbAddCustomerClicked();
    void onTbAddProjectClicked();
    void onTbAddActivityClicked();

    void onBtStartStopClicked();

    void onSecondTimeout();

    void onSessionCurrentTimeSheetChanged();
    void onSessionCacheSynchronizeFinished();

    void updateControls();

    void updateCustomersCombo();
    void updateProjectsCombo();
    void updateActivitiesCombo();

    Ui::ActivityWidget* mUi;
    QTimer mSecondTimer;
    std::shared_ptr<KemaiSession> mSession;
};

} // namespace kemai
