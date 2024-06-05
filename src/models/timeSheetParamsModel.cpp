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
                m_timeSheetParams.emplace_back(TimeSheetParams{
                    .customer = customer.name, .project = project.name, .activity = activity.name, .projectId = project.id, .activityId = activity.id});
            }
        }
    }

    endResetModel();
}

QVariant TimeSheetParamsModel::data(const QModelIndex& index, int role) const
{

    if (!index.isValid() || (index.row() > m_timeSheetParams.size()))
    {
        return {};
    }

    const auto& params = m_timeSheetParams.at(index.row());
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
    return static_cast<int>(m_timeSheetParams.size());
}
