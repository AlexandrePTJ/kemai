#include "taskfilterproxymodel.h"
#include "tasklistmodel.h"

using namespace kemai::models;

void TaskFilterProxyModel::setUserId(int userId)
{
    mUserId = userId;
    invalidateFilter();
}

bool TaskFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    const auto& index = sourceModel()->index(source_row, 0, source_parent);
    return sourceModel()->data(index, Qt::DisplayRole).toString().contains(filterRegularExpression()) &&
           sourceModel()->data(index, TaskListModel::UserIdRole).toInt() == mUserId;
}
