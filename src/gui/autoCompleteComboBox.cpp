#include "autoCompleteComboBox.h"

#include <QCompleter>

using namespace kemai;

class kemai::AutoCompleteValidator : public QValidator
{
public:
    explicit AutoCompleteValidator(AutoCompleteListModel* model) : mModel(model) {}
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
    AutoCompleteListModel* mModel = nullptr;
};

int AutoCompleteListModel::rowCount(const QModelIndex& parent) const
{
    return static_cast<int>(mData.size());
}

QVariant AutoCompleteListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() > mData.size())
    {
        return {};
    }

    const auto& it = std::next(mData.begin(), index.row());
    switch (role)
    {
    case Qt::EditRole:
    case Qt::DisplayRole:
        return it->first;

    case Qt::UserRole:
        return it->second;

    default:
        return {};
    }
}

AutoCompleteComboBox::AutoCompleteComboBox(QWidget* parent) : QComboBox(parent)
{
    setEditable(true);

    auto completer = new QCompleter(&mModel);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    setCompleter(completer);

    setValidator(new AutoCompleteValidator(&mModel));
}

AutoCompleteComboBox::~AutoCompleteComboBox() = default;
