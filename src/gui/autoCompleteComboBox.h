#pragma once

#include <QAbstractListModel>
#include <QComboBox>

namespace kemai {

class AutoCompleteListModel : public QAbstractListModel
{
public:
    int rowCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;

    template<class K> void setKimaiData(const std::vector<K>& kds)
    {
        beginResetModel();
        if (!kds.empty())
        {
            mData = {{"", 0}};
            for (const auto& kd : kds)
            {
                mData.emplace(kd.name, kd.id);
            }
        }
        endResetModel();
    }

private:
    std::map<QString, int> mData;
};

class AutoCompleteValidator;

class AutoCompleteComboBox : public QComboBox
{
    Q_OBJECT

public:
    AutoCompleteComboBox(QWidget* parent = nullptr);
    ~AutoCompleteComboBox() override;

    template<class K> void setKimaiData(const std::vector<K>& kds)
    {
        if (!mModelSet) // Avoid to call virtual setModel in ctor and to have to call explicitly somewhere else.
        {
            mModelSet=true;
            setModel(&mModel);
        }
        mModel.setKimaiData(kds);
    }

private:
    bool mModelSet=false;
    AutoCompleteListModel mModel;
    AutoCompleteValidator* mValidator = nullptr;
};

} // namespace kemai
