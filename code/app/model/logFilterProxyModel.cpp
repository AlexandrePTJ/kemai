// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "logFilterProxyModel.h"

// STL headers
#include <algorithm>

// Qt headers
#include <QStringList>

// Project headers
#include <model/logEntryModel.h>

namespace kemai
{
    LogFilterProxyModel::LogFilterProxyModel(QObject *parent):
    QSortFilterProxyModel(parent)
    {
        setDynamicSortFilter(true);
        connect(this, &QAbstractItemModel::rowsInserted, this, &LogFilterProxyModel::countChanged);
        connect(this, &QAbstractItemModel::rowsRemoved, this, &LogFilterProxyModel::countChanged);
        connect(this, &QAbstractItemModel::modelReset, this, &LogFilterProxyModel::countChanged);
        connect(this, &QAbstractItemModel::layoutChanged, this, &LogFilterProxyModel::countChanged);
    }

    LogFilterProxyModel::~LogFilterProxyModel() = default;

    int LogFilterProxyModel::minLevel() const
    {
        return m_minLevel;
    }

    void LogFilterProxyModel::setMinLevel(int level)
    {
        if (m_minLevel == level)
        {
            return;
        }
        beginFilterChange();
        m_minLevel = level;
        endFilterChange(QSortFilterProxyModel::Direction::Rows);
        emit minLevelChanged();
    }

    int LogFilterProxyModel::count() const
    {
        return rowCount();
    }

    QString LogFilterProxyModel::text(int first, int last) const
    {
        const auto *entries = qobject_cast<const LogEntryModel *>(sourceModel());
        if (entries == nullptr)
        {
            return {};
        }

        QStringList lines;
        for (int row = std::max(0, first); row <= std::min(last, rowCount() - 1); ++row)
        {
            lines.append(LogEntryModel::textLine(entries->entry(mapToSource(index(row, 0)).row())));
        }
        return lines.join('\n');
    }

    bool LogFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
    {
        const auto level = sourceModel()->index(sourceRow, 0, sourceParent).data(static_cast<int>(LogEntryModel::Role::Level)).toInt();
        return level >= m_minLevel;
    }
} // namespace kemai
