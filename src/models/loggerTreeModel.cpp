#include "loggerTreeModel.h"

#include <QColor>

using namespace kemai;

const auto MaxLogEntries = 500;

/*
 * Static helpers
 */
static QVariant getDisplay(const LoggerEntry& entry, int column)
{
    switch (column)
    {
    case 0:
        return entry.level;

    case 1:
        return entry.dateTime;

    case 2:
        return entry.message;

    default:
        return {};
    }
}

static QVariant getForeground(spdlog::level::level_enum level)
{
    switch (level)
    {
    case spdlog::level::trace:
    case spdlog::level::debug:
        return QColor(Qt::blue);

    case spdlog::level::info:
        return QColor(Qt::darkGreen);

    case spdlog::level::warn:
        return QColor(Qt::darkYellow);

    case spdlog::level::err:
    case spdlog::level::critical:
        return QColor(Qt::darkRed);

    default:
        return {};
    }
}

/*
 * Public impl
 */
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
    return static_cast<int>(mEntries.size());
}

int LoggerTreeModel::columnCount(const QModelIndex& parent) const
{
    return 3;
}

QVariant LoggerTreeModel::data(const QModelIndex& index, int role) const
{
    if (index.row() >= mEntries.size() || !index.isValid())
    {
        return {};
    }

    auto it = std::next(mEntries.begin(), index.row());
    switch (role)
    {
    case Qt::DisplayRole:
        return getDisplay(*it, index.column());

    case Qt::ForegroundRole:
        return getForeground(it->level);

    default:
        return {};
    }
}

QVariant LoggerTreeModel::headerData(int section, Qt::Orientation /*orientation*/, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return {};
    }

    switch (section)
    {
    case 0:
        return tr("Level");

    case 1:
        return tr("Date");

    case 2:
        return tr("Message");
    }

    return {};
}

void LoggerTreeModel::sinkLog(const LoggerEntry& entry)
{
    QMutexLocker lock(&mSinkMutex);

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
