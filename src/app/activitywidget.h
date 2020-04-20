#pragma once

#include <QTimer>
#include <QWidget>

#include "kemai/kimaiclient.h"

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

    void onBtStartStopClicked();

    void onSecondTimeout();

    void updateControls();

private:
    Ui::ActivityWidget* mUi;
    QTimer mSecondTimer;
    QScopedPointer<client::KimaiClient> mClient;
    QScopedPointer<client::TimeSheet> mCurrentTimeSheet;
};

} // namespace kemai::app
