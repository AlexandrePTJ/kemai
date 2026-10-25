// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// STL headers
#include <deque>

// Qt headers
#include <QAbstractListModel>
#include <QDateTime>
#include <QString>

namespace kemai
{
    /*!
     * Log entries of the current run, oldest first, for the log journal. Keeps the last `capacity` entries:
     * older ones are dropped (the log files keep everything).
     */
    class LogEntryModel : public QAbstractListModel
    {
        Q_OBJECT

        Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
        Q_PROPERTY(int errorCount READ errorCount NOTIFY countChanged)
        Q_PROPERTY(int warningCount READ warningCount NOTIFY countChanged)

    public:
        static constexpr int DefaultCapacity = 5000;

        // Ordered: a minimum level filter keeps a level and the ones after it
        enum class Level
        {
            Debug,
            Info,
            Warning,
            Error
        };
        Q_ENUM(Level)

        enum class Role
        {
            Time = Qt::UserRole + 1, // "HH:mm:ss"
            Level,                   // int, Level
            Message
        };

        struct Entry
        {
            QDateTime time;
            Level     level = Level::Info;
            QString   message;
        };

        explicit LogEntryModel(QObject *parent = nullptr, int capacity = DefaultCapacity);

        void         append(const Entry &entry);
        const Entry &entry(int row) const;

        int errorCount() const;
        int warningCount() const;

        // One line per entry for a bug report: "HH:mm:ss LEVEL   message", the message never truncated
        static QString textLine(const Entry &entry);

        int                    rowCount(const QModelIndex &parent = {}) const override;
        QVariant               data(const QModelIndex &index, int role) const override;
        QHash<int, QByteArray> roleNames() const override;

    signals:
        void countChanged();

    private:
        void count(Level level, int delta);

        std::deque<Entry> m_entries;
        int               m_capacity;
        int               m_errorCount   = 0;
        int               m_warningCount = 0;
    };
} // namespace kemai
