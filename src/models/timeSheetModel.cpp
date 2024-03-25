#include "timeSheetModel.h"

using namespace kemai;

void TimeSheetModel::setTimeSheets(const TimeSheets& timeSheets)
{
    beginResetModel();
    mTimeSheets = timeSheets;
    endResetModel();
}

int TimeSheetModel::rowCount(const QModelIndex& parent) const
{
    return static_cast<int>(mTimeSheets.size());
}

QVariant TimeSheetModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() > mTimeSheets.size())
    {
        return {};
    }

    const auto& it = std::next(mTimeSheets.begin(), index.row());
    switch (role)
    {
    case ActivityNameRole:
        return it->activity.name;
    case ProjectNameRole:
        return it->project.name;
    case TagsRole:
        return it->tags;
    case StartDateRole:
        return it->beginAt;
    case EndDateRole:
        return it->endAt;
    case DurationRole: {
        if (it->beginAt.isValid() && it->endAt.isValid())
        {
            return it->beginAt.secsTo(it->endAt);
        }
        return {};
    }

    default:
        return {};
    }
}

QHash<int, QByteArray> TimeSheetModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ActivityNameRole] = "activityName";
    roles[ProjectNameRole]  = "projectName";
    roles[TagsRole]         = "tags";
    roles[StartDateRole]    = "startDate";
    roles[EndDateRole]      = "endDate";
    roles[DurationRole]     = "duration";
    return roles;
}
