#include "kimaiDataListModel.h"

using namespace kemai;

int KimaiDataListModel::rowCount(const QModelIndex& parent) const
{
    return static_cast<int>(mData.size());
}

QVariant KimaiDataListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() > mData.size())
    {
        return {};
    }

    const auto& it = std::next(mData.begin(), index.row());
    switch (role)
    {
    case Qt::EditRole:
    case Qt::DisplayRole:
        return it->second;

    case Qt::UserRole:
        return it->first;

    default:
        return {};
    }
}

