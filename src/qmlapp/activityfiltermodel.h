#pragma once

#include <optional>

#include <QSortFilterProxyModel>

namespace kemai::qmlapp {

class ActivityFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(int projectId READ projectId WRITE setProjectId)

public:
    ActivityFilterModel(QObject* parent = nullptr);

    int projectId() const;
    void setProjectId(int id);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
    std::optional<int> mProjectId;
};

} // namespace kemai::qmlapp
