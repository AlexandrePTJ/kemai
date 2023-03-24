#pragma once

#include <vector>

#include <QSortFilterProxyModel>

namespace kemai {

class KimaiDataSortFilterProxyModel : public QSortFilterProxyModel
{
public:
    KimaiDataSortFilterProxyModel();
    ~KimaiDataSortFilterProxyModel() override;

    template<class K> void setKimaiFilter(const std::vector<K>& kds)
    {
        mIds.clear();
        for (const auto& k : kds)
        {
            mIds.push_back(k.id);
        }
        invalidateFilter();
    }

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
    std::vector<int> mIds;
};

} // namespace kemai
