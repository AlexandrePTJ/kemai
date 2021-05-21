#pragma once

#include <QTimer>
#include <QWidget>

#include "client/kimaiclient.h"

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

signals:
    void currentActivityChanged(bool started);

private:
    void onClientReply(const client::KimaiReply& reply);

    void onCbCustomerTextChanged(const QString& text);
    void onCbProjectTextChanged(const QString& text);
    void onCbActivityTextChanged(const QString& text);

    void onTbAddCustomerClicked();
    void onTbAddProjectClicked();
    void onTbAddActivityClicked();

    void onBtStartStopClicked();

    void onSecondTimeout();

    void updateControls();

private:
    Ui::ActivityWidget* mUi;
    QTimer mSecondTimer;
    QSharedPointer<client::KimaiClient> mClient;
    QScopedPointer<client::TimeSheet> mCurrentTimeSheet;
    QScopedPointer<client::User> mMe;
};

} // namespace kemai::app
