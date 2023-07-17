#include "autoCompleteComboBox.h"

#include <QCompleter>

using namespace kemai;

class kemai::AutoCompleteValidator : public QValidator
{
public:
    explicit AutoCompleteValidator(KimaiDataListModel* model) : mModel(model) {}
    ~AutoCompleteValidator() override = default;

    QValidator::State validate(QString& input, int& /*pos*/) const override
    {
        auto nRows = mModel->rowCount({});

        if (input.isEmpty() || nRows == 0)
        {
            return Acceptable;
        }

        for (auto i = 0; i < nRows; ++i)
        {
            auto str = mModel->data(mModel->index(i), Qt::DisplayRole).toString();
            if (str == input)
            {
                return Acceptable;
            }
            if (str.contains(input, Qt::CaseInsensitive))
            {
                return Intermediate;
            }
        }

        return Invalid;
    }

    void fixup(QString& input) const override { input.clear(); }

private:
    KimaiDataListModel* mModel;
};

AutoCompleteComboBox::AutoCompleteComboBox(QWidget* parent) : QComboBox(parent)
{
    mProxyModel.setSourceModel(&mModel);

    setEditable(true);

    auto completer = new QCompleter(&mModel);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    setCompleter(completer);

    setValidator(new AutoCompleteValidator(&mModel));
}

AutoCompleteComboBox::~AutoCompleteComboBox() = default;
