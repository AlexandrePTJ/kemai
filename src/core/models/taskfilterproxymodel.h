#pragma once

#include <QSortFilterProxyModel>

namespace kemai::models {

class TaskFilterProxyModel : public QSortFilterProxyModel
{
public:
    TaskFilterProxyModel();
    ~TaskFilterProxyModel() override;

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
};

} // namespace kemai::models
