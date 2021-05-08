#pragma once

#include <optional>

#include <QSortFilterProxyModel>

namespace kemai::qmlapp {

class ProjectFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(int customerId READ customerId WRITE setCustomerId)

public:
    ProjectFilterModel(QObject* parent = nullptr);

    int customerId() const;
    void setCustomerId(int id);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
    std::optional<int> mCustomerId;
};

} // namespace kemai::qmlapp
