#pragma once

#include <QTimer>
#include <QWidget>

#include "client/kimaiclient.h"
#include "context/kemaisession.h"

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

    void setKimaiClient(QSharedPointer<KimaiClient> kimaiClient);
    void setKemaiSession(QSharedPointer<KemaiSession> kemaiSession);

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

    void processActiveTimeSheetsResult(TimeSheetsResult timeSheetsResult);
    void processCustomersResult(CustomersResult customersResult);
    void processProjectsResult(ProjectsResult projectsResult);
    void processActivitiesResult(ActivitiesResult activitiesResult);
    void processTimeSheetResult(TimeSheetResult timeSheetResult);

    Ui::ActivityWidget* mUi;
    QTimer mSecondTimer;
    QSharedPointer<KimaiClient> mClient;
    QSharedPointer<KemaiSession> mSession;
    QScopedPointer<TimeSheet> mCurrentTimeSheet;
};

} // namespace kemai
