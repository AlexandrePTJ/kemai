#pragma once

#include <QWidget>

#include "client/kimaiclient.h"
#include "context/kemaisession.h"
#include "models/taskfilterproxymodel.h"
#include "models/tasklistmodel.h"

namespace Ui {
class TaskWidget;
}

namespace kemai {

class TaskWidget : public QWidget
{
    Q_OBJECT

public:
    TaskWidget(QWidget* parent = nullptr);
    ~TaskWidget() override;

    void setKimaiClient(QSharedPointer<KimaiClient> kimaiClient);
    void setKemaiSession(QSharedPointer<KemaiSession> kemaiSession);

signals:
    void taskStarted();
    void taskStopped();

private:
    void updateTasks();
    void onTaskItemChanged(const QModelIndex& current, const QModelIndex& previous);
    void onRefreshClicked();
    void onStartStopClicked();
    void onCloseClicked();

    Ui::TaskWidget* mUi;
    QSharedPointer<KimaiClient> mClient;
    TaskListModel mTaskModel;
    TaskFilterProxyModel mTaskProxyModel;
};

} // namespace kemai
