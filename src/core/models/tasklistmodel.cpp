#include "tasklistmodel.h"

using namespace kemai::client;
using namespace kemai::models;

TaskListModel::TaskListModel() : QAbstractListModel() {}

TaskListModel::~TaskListModel() = default;

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
        return QVariant();
    }

    const auto& task = mTasks.at(index.row());
    switch (role)
    {
    case Qt::DisplayRole:
        return QString("%1\n- %2").arg(task.title, task.description);

    default:
        return QVariant();
    }
}

int TaskListModel::rowCount(const QModelIndex& /*parent*/) const
{
    return mTasks.size();
}
