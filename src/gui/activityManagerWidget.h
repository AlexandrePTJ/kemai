#pragma once

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
    void onAddActivityWidgetButtonClicked();
    void onSessionCacheSynchronizeFinished();

    Ui::ActivityManagerWidget* mUi;
    QToolButton* mAddActivityWidgetButton = nullptr;

    std::shared_ptr<KemaiSession> mSession;
};

} // namespace kemai
