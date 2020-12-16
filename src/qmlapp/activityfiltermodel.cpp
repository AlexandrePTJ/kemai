#include "activityfiltermodel.h"

#include "kimaibasedatamodel.h"

using namespace kemai::qmlapp;

ActivityFilterModel::ActivityFilterModel(QObject* parent) : QSortFilterProxyModel(parent) {}

int ActivityFilterModel::projectId() const
{
    return mProjectId.value_or(0);
}

void ActivityFilterModel::setProjectId(int id)
{
    mProjectId = id;
    invalidateFilter();
}

bool ActivityFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    if (!mProjectId.has_value())
        return true;

    auto index = sourceModel()->index(sourceRow, 0);
    auto vid   = sourceModel()->data(index, ProjectIdRole);
    if (vid.isValid())
    {
        return vid.toInt() == mProjectId.value();
    }
    return true;
}
