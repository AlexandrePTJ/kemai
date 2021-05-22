#pragma once

#include <QAbstractListModel>

#include "client/kimaiapi.h"

namespace kemai::models {

class TaskListModel : public QAbstractListModel
{
public:
    TaskListModel();
    ~TaskListModel() override;

    void setTasks(const client::Tasks& tasks);

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

private:
    client::Tasks mTasks;
};

} // namespace kemai::models
