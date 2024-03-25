#pragma once

#include <QAbstractListModel>

#include "client/kimaiAPI.h"

namespace kemai {

class TimeSheetModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum TimeSheetRoles
    {
        ActivityNameRole = Qt::UserRole + 1,
        ProjectNameRole,
        TagsRole,
        StartDateRole,
        EndDateRole,
        DurationRole
    };

    void setTimeSheets(const TimeSheets& timeSheets);

    int rowCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const;

private:
    TimeSheets mTimeSheets;
};

} // namespace kemai
