#include "loggerTreeModel.h"

using namespace kemai;

const auto MaxLogEntries = 500;

LoggerTreeModel::LoggerTreeModel(QObject* parent) : QAbstractItemModel(parent) {}

LoggerTreeModel::~LoggerTreeModel() = default;

QModelIndex LoggerTreeModel::index(int row, int column, const QModelIndex& /*parent*/) const
{
    return createIndex(row, column);
}

QModelIndex LoggerTreeModel::parent(const QModelIndex& /*child*/) const
{
    return {};
}

int LoggerTreeModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return mEntries.size();
}

int LoggerTreeModel::columnCount(const QModelIndex& parent) const
{
    return 3;
}

QVariant LoggerTreeModel::data(const QModelIndex& index, int role) const
{
    if (index.row() >= mEntries.size() || !index.isValid() || role != Qt::DisplayRole)
    {
        return {};
    }

    auto it = std::next(mEntries.begin(), index.row());
    switch (index.column())
    {
    case 0:
        return it->level;

    case 1:
        return it->dateTime;

    case 2:
        return it->message;

    default:
        return {};
    }
}

void LoggerTreeModel::sinkLog(const LoggerEntry& entry)
{
    if (entry.level <= spdlog::level::debug)
    {
        return;
    }

    const auto size = static_cast<int>(mEntries.size());
    if (size > MaxLogEntries - 1)
    {
        beginMoveRows({}, 1, size - 1, {}, 0);
        mEntries.push_back(entry);
        mEntries.pop_front();
        endMoveRows();
    }
    else
    {
        beginInsertRows({}, size, size + 1);
        mEntries.push_back(entry);
        endInsertRows();
    }
}
