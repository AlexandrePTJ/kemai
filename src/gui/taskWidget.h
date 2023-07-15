#pragma once

#include <QWidget>

#include "client/kimaiClient.h"
#include "context/kemaiSession.h"
#include "models/taskFilterProxyModel.h"
#include "models/taskListModel.h"

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

    void setKemaiSession(std::shared_ptr<KemaiSession> kemaiSession);

private:
    void updateTasks();
    void onTaskItemChanged(const QModelIndex& current, const QModelIndex& previous);
    void onStartStopClicked();
    void onCloseClicked();

    std::unique_ptr<Ui::TaskWidget> mUi;
    std::shared_ptr<KemaiSession> mSession;
    TaskListModel mTaskModel;
    TaskFilterProxyModel mTaskProxyModel;
};

} // namespace kemai
