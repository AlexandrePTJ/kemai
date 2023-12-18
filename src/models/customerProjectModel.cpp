#include "customerProjectModel.h"

#include <QBrush>
#include <QFont>

using namespace kemai;

class CustomerProjectModel::CustomerProjectItem
{
public:
    CustomerProjectItem() = default;

    CustomerProjectItem(int id, const QString& label, CustomerProjectItem* parent = nullptr) : mId(id), mLabel(label), mParent(parent) {}

    const QString& label() const { return mLabel; }
    int id() const { return mId; }

    bool isRoot() const { return mParent == nullptr; }

    bool isCustomer() const
    {
        if (mParent != nullptr)
        {
            return mParent->isRoot();
        }
        return false;
    }

    bool isProject() const
    {
        if (mParent != nullptr)
        {
            return mParent->isCustomer();
        }
        return false;
    }

    int getChildCount() { return static_cast<int>(mChildren.size()); }

    CustomerProjectItem* getChildAt(int index)
    {
        if (index < 0 || index >= static_cast<int>(mChildren.size()))
        {
            return nullptr;
        }
        return mChildren.at(index).get();
    }

    int appendChild(std::unique_ptr<CustomerProjectItem> child)
    {
        mChildren.push_back(std::move(child));
        mChildren.back()->setParent(this);
        return static_cast<int>(mChildren.size() - 1);
    }

    void setParent(CustomerProjectItem* parent) { mParent = parent; }

private:
    const int mId = -1;
    const QString mLabel;
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

    mRootItem = std::make_unique<CustomerProjectItem>();

    for (const auto& customer : customers)
    {
        auto customerIndex = mRootItem->appendChild(std::make_unique<CustomerProjectItem>(customer.id, customer.name));
        auto* customerItem = mRootItem->getChildAt(customerIndex);

        mFlatItems.emplace_back(*customerItem);

        for (const auto& project : projects)
        {
            if (project.customer.id == customer.id)
            {
                auto projectIndex = customerItem->appendChild(std::make_unique<CustomerProjectItem>(project.id, project.name));
                mFlatItems.emplace_back(*customerItem->getChildAt(projectIndex));
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
        case Qt::DisplayRole:
            return QString("  %1").arg(item.label());

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
