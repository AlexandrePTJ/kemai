#include "tasklistmodel.h"

using namespace kemai::client;
using namespace kemai::models;

void TaskListModel::setTasks(const Tasks& tasks)
{
    beginResetModel();
    mTasks = tasks;
    endResetModel();
}

QVariant TaskListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || (index.row() > mTasks.size()))
    {
        return {};
    }

    const auto& task = mTasks.at(index.row());
    switch (role)
    {
    case Qt::DisplayRole:
        return QString("%1\n- %2").arg(task.title, task.description);

    case TaskIDRole:
        return task.id;

    default:
        return {};
    }
}

int TaskListModel::rowCount(const QModelIndex& /*parent*/) const
{
    return static_cast<int>(mTasks.size());
}
