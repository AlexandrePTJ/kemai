#include "customerProjectModel.h"

#include <QBrush>
#include <QFont>

using namespace kemai;

class CustomerProjectModel::CustomerProjectItem
{
public:
    CustomerProjectItem() = default; // only used for root item

    CustomerProjectItem(int id, const QString& label, bool isCustomer) : mId(id), mLabel(label), mIsCustomer(isCustomer) {}

    const QString& label() const { return mLabel; }
    int id() const { return mId; }

    QString fullLabel() const
    {
        if (mParent != nullptr)
        {
            auto parentFullLabel = mParent->fullLabel();
            if (!parentFullLabel.isEmpty())
            {
                return QString("%1 / %2").arg(parentFullLabel, mLabel);
            }
        }
        return mLabel;
    }

    bool isRoot() const { return mParent == nullptr; }

    bool isCustomer() const { return !isRoot() && mIsCustomer; }

    bool isProject() const { return !isRoot() && !mIsCustomer; }

    int getChildCount() const { return static_cast<int>(mChildren.size()); }

    CustomerProjectItem* getChildAt(int index) const
    {
        if (index < 0 || index >= static_cast<int>(mChildren.size()))
        {
            return nullptr;
        }
        return mChildren.at(index).get();
    }

    CustomerProjectItem* appendChild(int id, const QString& label, bool isCustomer = false)
    {
        auto child     = std::make_unique<CustomerProjectItem>(id, label, isCustomer);
        child->mParent = this;

        mChildren.push_back(std::move(child));
        return mChildren.back().get();
    }

private:
    const int mId = -1;
    const QString mLabel;
    const bool mIsCustomer       = false;
    CustomerProjectItem* mParent = nullptr;
    std::vector<std::unique_ptr<CustomerProjectItem>> mChildren;
};

CustomerProjectModel::CustomerProjectModel(QObject* parent)
    : QAbstractListModel(parent), mRootItem(std::make_unique<CustomerProjectModel::CustomerProjectItem>())
{}

CustomerProjectModel::~CustomerProjectModel() = default;

void CustomerProjectModel::setCustomersProjects(const Customers& customers, const Projects& projects)
{
    beginResetModel();

    mFlatItems.clear();
    mRootItem = std::make_unique<CustomerProjectItem>();

    auto emptyItem = mRootItem->appendChild(-1, "");
    mFlatItems.emplace_back(*emptyItem);

    for (const auto& customer : customers)
    {
        auto customerItem = mRootItem->appendChild(customer.id, customer.name, true);
        mFlatItems.emplace_back(*customerItem);

        for (const auto& project : projects)
        {
            if (project.customer.id == customer.id)
            {
                auto projectItem = customerItem->appendChild(project.id, project.name);
                mFlatItems.emplace_back(*projectItem);
            }
        }
    }

    endResetModel();
}

int CustomerProjectModel::rowCount(const QModelIndex& parent) const
{
    int nbItems = mRootItem->getChildCount();

    for (int idx = 0; idx < mRootItem->getChildCount(); ++idx)
    {
        nbItems += mRootItem->getChildAt(idx)->getChildCount();
    }

    return nbItems;
}

QVariant CustomerProjectModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }

    const auto& item = getItemAt(index.row());

    if (item.isRoot())
    {
        return {};
    }

    if (item.isCustomer() && index.column() == 0)
    {
        switch (role)
        {
        case Qt::DisplayRole:
            return item.label();

        case Qt::FontRole: {
            QFont font;
            font.setBold(true);

            return font;
        }

        case Qt::ForegroundRole:
            return QBrush(Qt::gray);

        default:
            return {};
        }
    }

    if (item.isProject())
    {
        switch (role)
        {
        case Qt::EditRole:
            return item.fullLabel();

        case Qt::DisplayRole:
            return QString("  %1").arg(item.label());

        case Qt::UserRole:
            return item.id();

        default:
            return {};
        }
    }

    return {};
}

Qt::ItemFlags CustomerProjectModel::flags(const QModelIndex& index) const
{
    auto f = QAbstractListModel::flags(index);

    if (index.isValid())
    {
        auto rowIdx = index.row();
        if (rowIdx == rowCount(index) || (rowIdx < rowCount(index) && getItemAt(rowIdx).isCustomer()))
        {
            // Don't want customer items to be selectable nor enabled
            f &= ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        }
    }

    return f;
}

const CustomerProjectModel::CustomerProjectItem& CustomerProjectModel::getItemAt(int index) const
{
    return mFlatItems.at(index);
}
