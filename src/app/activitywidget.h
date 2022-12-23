#pragma once

#include <QTimer>
#include <QWidget>

#include "client/kimaiclient.h"
#include "kemaisession.h"

namespace Ui {
class ActivityWidget;
}

namespace kemai::app {

class ActivityWidget : public QWidget
{
    Q_OBJECT

public:
    ActivityWidget(QWidget* parent = nullptr);
    ~ActivityWidget() override;

    void setKimaiClient(QSharedPointer<client::KimaiClient> kimaiClient);
    void setKemaiSession(QSharedPointer<core::KemaiSession> kemaiSession);

signals:
    void currentActivityChanged(bool started);

private:
    void onCbCustomerTextChanged(const QString& text);
    void onCbProjectTextChanged(const QString& text);
    void onCbActivityTextChanged(const QString& text);

    void onTbAddCustomerClicked();
    void onTbAddProjectClicked();
    void onTbAddActivityClicked();

    void onBtStartStopClicked();

    void onSecondTimeout();

    void updateControls();

    void processCustomersResult(client::CustomersResult customersResult);
    void processActiveTimeSheetsResult(client::TimeSheetsResult timeSheetsResult);
    void processProjectsResult(client::ProjectsResult projectsResult);
    void processActivitiesResult(client::ActivitiesResult activitiesResult);
    void processTimeSheetResult(client::TimeSheetResult timeSheetResult);

    Ui::ActivityWidget* mUi;
    QTimer mSecondTimer;
    QSharedPointer<client::KimaiClient> mClient;
    QSharedPointer<core::KemaiSession> mSession;
    QScopedPointer<client::TimeSheet> mCurrentTimeSheet;
};

} // namespace kemai::app

