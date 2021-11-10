#pragma once

#include <QSortFilterProxyModel>

namespace kemai::models {

class TaskFilterProxyModel : public QSortFilterProxyModel
{
public:
    void setUserId(int userId);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
    int mUserId = 0;
};

} // namespace kemai::models
