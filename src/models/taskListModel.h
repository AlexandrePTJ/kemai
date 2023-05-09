#pragma once

#include <QAbstractListModel>

#include "client/kimaiAPI.h"

namespace kemai {

class TaskListModel : public QAbstractListModel
{
public:
    enum TaskModelRole
    {
        TaskIDRole = Qt::UserRole + 1,
        UserIdRole
    };

    void setTasks(const Tasks& tasks);

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

private:
    Tasks mTasks;
};

} // namespace kemai
