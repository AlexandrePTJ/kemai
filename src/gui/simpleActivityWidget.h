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

    void updateProjectsCombo();
    void updateActivitiesCombo();
    void updateTimeSheets();

    std::unique_ptr<Ui::SimpleActivityWidget> mUi;
    std::shared_ptr<KemaiSession> mSession;
    CustomerProjectModel mCustomerProjectModel;
};

} // namespace kemai
