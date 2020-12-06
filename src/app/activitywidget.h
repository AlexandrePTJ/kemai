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
    ~ActivityWidget();

public slots:
    void refresh();

private slots:
    void onClientError(const QString& errorMsg);
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

signals:
    void currentActivityChanged(bool started);

private:
    Ui::ActivityWidget* mUi;
    QTimer mSecondTimer;
    QSharedPointer<client::KimaiClient> mClient;
    QScopedPointer<client::TimeSheet> mCurrentTimeSheet;
    QScopedPointer<client::User> mMe;
};

} // namespace kemai::app
