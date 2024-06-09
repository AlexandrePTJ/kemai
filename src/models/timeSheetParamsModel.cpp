#include "timeSheetParamsModel.h"

using namespace kemai;

void TimeSheetParamsModel::updateDataFromCache(const KimaiCache& cache)
{
    beginResetModel();

    for (const auto& customer : cache.customers())
    {
        for (const auto& project : cache.projects(customer.id))
        {
            for (const auto& activity : cache.activities(project.id))
            {
                mTimeSheetParams.emplace_back(TimeSheetParams{
                    .customer = customer.name, .project = project.name, .activity = activity.name, .projectId = project.id, .activityId = activity.id});
            }
        }
    }

    endResetModel();
}

int TimeSheetParamsModel::findIndex(int projectId, int activityId) const
{
    auto it = std::find_if(mTimeSheetParams.begin(), mTimeSheetParams.end(), [projectId, activityId](const auto& timeSheetParams) {
        return timeSheetParams.projectId == projectId && timeSheetParams.activityId == activityId;
    });
    if (it == mTimeSheetParams.end())
    {
        return -1;
    }
    return std::distance(mTimeSheetParams.begin(), it);
}

QVariant TimeSheetParamsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || (index.row() > mTimeSheetParams.size()))
    {
        return {};
    }

    const auto& params = mTimeSheetParams.at(index.row());
    switch (role)
    {
    case Qt::DisplayRole:
        return QString("%1 / %2 / %3").arg(params.customer, params.project, params.activity);

    case CustomerName:
        return params.customer;

    case ProjectName:
        return params.project;

    case ActivityName:
        return params.activity;

    case ProjectIdRole:
        return params.projectId;

    case ActivityIdRole:
        return params.activityId;

    default:
        return {};
    }
}

int TimeSheetParamsModel::rowCount(const QModelIndex& /*parent*/) const
{
    return static_cast<int>(mTimeSheetParams.size());
}
