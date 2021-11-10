#include "tasklistmodel.h"

#include <QApplication>
#include <QStyle>

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
    case TaskIDRole:
        return task.id;

    case UserIdRole:
        return task.user.id;

    case Qt::DisplayRole: {
        auto display = task.title;
        if (!task.description.isEmpty())
        {
            display += "\n- " + task.description;
        }
        return display;
    }

    case Qt::DecorationRole:
        return task.activeTimeSheets.isEmpty() ? QIcon() : qApp->style()->standardIcon(QStyle::SP_ArrowRight);

    default:
        return {};
    }
}

int TaskListModel::rowCount(const QModelIndex& /*parent*/) const
{
    return static_cast<int>(mTasks.size());
}
