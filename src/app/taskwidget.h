#pragma once

#include <QWidget>

#include "client/kimaiclient.h"

namespace Ui {
class TaskWidget;
}

namespace kemai::app {

class TaskWidget : public QWidget
{
    Q_OBJECT

public:
    TaskWidget(QWidget* parent = nullptr);
    ~TaskWidget() override;

    void setKimaiClient(QSharedPointer<client::KimaiClient> kimaiClient);

private:
    void onClientReply(const client::KimaiReply& reply);

private:
    Ui::TaskWidget* mUi;
    QSharedPointer<client::KimaiClient> mClient;
};

} // namespace kemai::app
