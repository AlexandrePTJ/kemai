#pragma once

#include <QAbstractListModel>

namespace kemai {

class KimaiDataListModel : public QAbstractListModel
{
public:
    int rowCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;

    template<class K> void setKimaiData(const std::vector<K>& kds)
    {
        beginResetModel();
        if (!kds.empty())
        {
            mData = {{0, ""}};
            for (const auto& kd : kds)
            {
                mData.emplace_back(std::make_pair(kd.id, kd.name));
            }
            std::sort(mData.begin(), mData.end(), [](const auto& a, const auto& b) { return a.second.toLower() < b.second.toLower(); });
        }
        endResetModel();
    }

private:
    std::vector<std::pair<int, QString>> mData;
};

} // namespace kemai
