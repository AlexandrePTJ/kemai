#include "kimaiDataSortFilterProxyModel.h"

using namespace kemai;

KimaiDataSortFilterProxyModel::KimaiDataSortFilterProxyModel() = default;

KimaiDataSortFilterProxyModel::~KimaiDataSortFilterProxyModel() = default;

bool KimaiDataSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    if (mIds.empty())
    {
        return true;
    }

    auto index = sourceModel()->index(sourceRow, 0, sourceParent);
    auto id    = sourceModel()->data(index, Qt::UserRole).toInt();
    auto name  = sourceModel()->data(index, Qt::DisplayRole).toString();

    // Show filtered values and empty field
    return std::find(mIds.begin(), mIds.end(), id) != mIds.end() || name.isEmpty();
}
