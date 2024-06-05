#pragma once

#include <QAbstractListModel>

#include <client/kimaiCache.h>

namespace kemai {

class TimeSheetParamsModel : public QAbstractListModel
{
public:
    enum TimeSheetParamsModelRole
    {
        CustomerName = Qt::UserRole + 1,
        ProjectName,
        ActivityName,
        ProjectIdRole,
        ActivityIdRole
    };

    void updateDataFromCache(const KimaiCache& cache);

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

private:
    struct TimeSheetParams
    {
        QString customer;
        QString project;
        QString activity;
        int projectId;
        int activityId;
    };
    std::vector<TimeSheetParams> m_timeSheetParams;
};

} // namespace kemai