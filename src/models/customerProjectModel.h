#pragma once

#include <QAbstractListModel>

#include "client/kimaiAPI.h"

namespace kemai {

class CustomerProjectModel : public QAbstractListModel
{
    class CustomerProjectItem;

public:
    CustomerProjectModel(QObject* parent = nullptr);
    ~CustomerProjectModel() override;

    void setCustomersProjects(const Customers& customers, const Projects& projects);

    int rowCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
    const CustomerProjectItem& getItemAt(int index) const;

    std::unique_ptr<CustomerProjectItem> mRootItem;
    std::vector<std::reference_wrapper<CustomerProjectItem>> mFlatItems;
};

} // namespace kemai
