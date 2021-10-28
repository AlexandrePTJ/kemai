#pragma once

#include <QAbstractListModel>

#include "client/kimaiapi.h"

namespace kemai::models {

class TaskListModel : public QAbstractListModel
{
public:
    enum TaskModelRole
    {
        TaskIDRole = Qt::UserRole + 1,
        UserIdRole
    };

    void setTasks(const client::Tasks& tasks);

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

private:
    client::Tasks mTasks;
};

} // namespace kemai::models
