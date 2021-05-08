#include "projectfiltermodel.h"

#include "kimaibasedatamodel.h"

using namespace kemai::qmlapp;

ProjectFilterModel::ProjectFilterModel(QObject* parent) : QSortFilterProxyModel(parent) {}

int ProjectFilterModel::customerId() const
{
    return mCustomerId.value_or(0);
}

void ProjectFilterModel::setCustomerId(int id)
{
    mCustomerId = id;
    invalidateFilter();
}

bool ProjectFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex&) const
{
    if (!mCustomerId.has_value())
        return true;

    auto index = sourceModel()->index(sourceRow, 0);
    auto vid   = sourceModel()->data(index, CustomerIdRole);
    if (vid.isValid())
    {
        return vid.toInt() == mCustomerId.value();
    }
    return false;
}
