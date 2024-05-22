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
        auto current = QVariant();
        if (!mModelSet) // Avoid to call virtual setModel in ctor and to have to call explicitly somewhere else.
        {
            mModelSet = true;
            setModel(&mProxyModel);
        }
        else
        {
            // Save which item was selected before the model reset
            current = currentData();
        }
        mModel.setKimaiData(kds);
        if (!current.isNull())
        {
            // Find the item with a matching ID and make it current again
            int foundIdx = findData(current, Qt::UserRole);
            if (foundIdx > 0)
            {
                setCurrentIndex(foundIdx);
            }
        }
    }

    template<class K> void setFilter(const std::vector<K>& kds) { mProxyModel.setKimaiFilter(kds); }

private:
    bool mModelSet = false;
    KimaiDataListModel mModel;
    KimaiDataSortFilterProxyModel mProxyModel;
};

} // namespace kemai
