#pragma once

#include <QWidget>

#include "client/kimaiClient.h"
#include "context/kemaiSession.h"
#include "models/customerProjectModel.h"

namespace Ui {
class SimpleActivityWidget;
}

namespace kemai {

class SimpleActivityWidget : public QWidget
{
    Q_OBJECT

public:
    SimpleActivityWidget(QWidget* parent = nullptr);
    ~SimpleActivityWidget() override;

    void setKemaiSession(std::shared_ptr<KemaiSession> kemaiSession);

private:
    void onCbProjectFieldChanged();
    void onCbActivityFieldChanged();

    void onBtCreateClicked();

    void onSessionCacheSynchronizeFinished();

    void onTimeSheetStartRequested(const TimeSheet& timeSheet);
    void onTimeSheetStopRequested(const TimeSheet& timeSheet);

    void onSecondTimeout();

    void updateControls();
    void updateTimeSheets();

    std::unique_ptr<Ui::SimpleActivityWidget> mUi;
    std::shared_ptr<KemaiSession> mSession;
    QTimer mSecondTimer;
    CustomerProjectModel mCustomerProjectModel;
};

} // namespace kemai
