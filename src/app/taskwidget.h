#pragma once

#include <QWidget>

#include "client/kimaiclient.h"
#include "models/taskfilterproxymodel.h"
#include "models/tasklistmodel.h"

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
    void onTaskItemChanged(const QModelIndex& current, const QModelIndex& previous);
    void onRefreshClicked();
    void onStartStopClicked();

private:
    Ui::TaskWidget* mUi;
    QSharedPointer<client::KimaiClient> mClient;
    models::TaskListModel mTaskModel;
    models::TaskFilterProxyModel mTaskProxyModel;
};

} // namespace kemai::app
