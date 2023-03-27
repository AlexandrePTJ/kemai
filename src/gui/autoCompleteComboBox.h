#pragma once

#include <QComboBox>

#include <models/kimaiDataListModel.h>
#include <models/kimaiDataSortFilterProxyModel.h>

namespace kemai {

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
            mModelSet = true;
            setModel(&mProxyModel);
        }
        mModel.setKimaiData(kds);
    }

    template<class K> void setFilter(const std::vector<K>& kds) { mProxyModel.setKimaiFilter(kds); }

private:
    bool mModelSet = false;
    KimaiDataListModel mModel;
    KimaiDataSortFilterProxyModel mProxyModel;
    AutoCompleteValidator* mValidator = nullptr;
};

} // namespace kemai
