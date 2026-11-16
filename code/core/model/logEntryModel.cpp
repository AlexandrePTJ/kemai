// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "logEntryModel.h"

// STL headers
#include <algorithm>

namespace kemai
{
    LogEntryModel::LogEntryModel(QObject *parent, int capacity):
    QAbstractListModel(parent),
    m_capacity(std::max(1, capacity))
    {
    }

    void LogEntryModel::append(const Entry &entry)
    {
        if (static_cast<int>(m_entries.size()) >= m_capacity)
        {
            beginRemoveRows({}, 0, 0);
            count(m_entries.front().level, -1);
            m_entries.pop_front();
            endRemoveRows();
        }

        const auto row = static_cast<int>(m_entries.size());
        beginInsertRows({}, row, row);
        m_entries.push_back(entry);
        count(entry.level, 1);
        endInsertRows();
        emit countChanged();
    }

    const LogEntryModel::Entry &LogEntryModel::entry(int row) const
    {
        return m_entries.at(static_cast<std::size_t>(row));
    }

    int LogEntryModel::errorCount() const
    {
        return m_errorCount;
    }

    int LogEntryModel::warningCount() const
    {
        return m_warningCount;
    }

    QString LogEntryModel::textLine(const Entry &entry)
    {
        const auto level = [&entry]
        {
            switch (entry.level)
            {
            case Level::Debug:
                return "DEBUG";
            case Level::Info:
                return "INFO";
            case Level::Warning:
                return "WARNING";
            case Level::Error:
                break;
            }
            return "ERROR";
        }();
        return QString("%1 %2 %3").arg(entry.time.toString("HH:mm:ss"), QString(level).leftJustified(7), entry.message);
    }

    int LogEntryModel::rowCount(const QModelIndex &parent) const
    {
        return parent.isValid() ? 0 : static_cast<int>(m_entries.size());
    }

    QVariant LogEntryModel::data(const QModelIndex &index, int role) const
    {
        if (!index.isValid() || index.row() >= rowCount())
        {
            return {};
        }

        const auto &e = entry(index.row());
        switch (static_cast<Role>(role))
        {
        case Role::Time:
            return e.time.toString("HH:mm:ss");
        case Role::Level:
            return static_cast<int>(e.level);
        case Role::Message:
            return e.message;
        }
        return {};
    }

    QHash<int, QByteArray> LogEntryModel::roleNames() const
    {
        return {
            {static_cast<int>(Role::Time),    "time"   },
            {static_cast<int>(Role::Level),   "level"  },
            {static_cast<int>(Role::Message), "message"}
        };
    }

    void LogEntryModel::count(Level level, int delta)
    {
        if (level == Level::Error)
        {
            m_errorCount += delta;
        }
        else if (level == Level::Warning)
        {
            m_warningCount += delta;
        }
    }
} // namespace kemai
