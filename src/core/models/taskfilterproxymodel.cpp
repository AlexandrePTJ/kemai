#include "taskfilterproxymodel.h"

using namespace kemai::models;

bool TaskFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    const auto& index = sourceModel()->index(source_row, 0, source_parent);
    return sourceModel()->data(index).toString().contains(filterRegularExpression());
}
