// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// STL headers
#include <optional>

// Qt headers
#include <QAbstractListModel>

// Project headers
#include <client/kimaiTimesheets.h>

namespace kemai
{
    /*!
     * Open tasks of the Task Management plugin, grouped for the tasks view: in progress (running first),
     * overdue (oldest due first), upcoming (nearest due first), no due date (by title). With a filter text,
     * the matching tasks form a single "matches" group.
     */
    class TaskModel : public QAbstractListModel
    {
        Q_OBJECT

        // Shown rows (after the filter)
        Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
        // All open tasks
        Q_PROPERTY(int totalCount READ totalCount NOTIFY countChanged)
        Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)
        // Title of the task the running timer belongs to, empty if none
        Q_PROPERTY(QString runningTaskTitle READ runningTaskTitle NOTIFY countChanged)

    public:
        enum class Role
        {
            TaskId = Qt::UserRole + 1,
            Title,
            Todo,
            Status,  // "pending" | "progress"
            Running, // the running timer belongs to it
            ProjectId,
            ProjectName,
            ActivityId,
            ActivityName,
            ActivityColor,
            DueAt,       // local QDateTime, invalid without due date
            DueState,    // "none" | "upcoming" | "today" | "overdue"
            OverdueDays, // whole days past the due date
            Estimation,  // seconds, 0 without estimate
            Group        // "progress" | "overdue" | "upcoming" | "none" | "matches"
        };

        explicit TaskModel(QObject *parent = nullptr);

        // `activeTimeSheetId`: the running timer (0 if none), to tell the running task
        void setTasks(const KimaiTasks &tasks, int activeTimeSheetId);
        void setActiveTimeSheetId(int activeTimeSheetId);
        void removeTask(int taskId);
        // A task by id, nullptr if it is not in the list
        const KimaiTask *findById(int taskId) const;

        int     totalCount() const;
        QString filterText() const;
        void    setFilterText(const QString &text);
        QString runningTaskTitle() const;

        // Shown rows in a group, for its header
        Q_INVOKABLE int groupCount(const QString &group) const;

        // For tests: today's date used to tell overdue tasks
        void setToday(const QDate &today);

        int                    rowCount(const QModelIndex &parent = {}) const override;
        QVariant               data(const QModelIndex &index, int role) const override;
        QHash<int, QByteArray> roleNames() const override;

    signals:
        void countChanged();
        void filterTextChanged();

    private:
        QString groupOf(const KimaiTask &task) const;
        QString dueStateOf(const KimaiTask &task) const;
        bool    isRunning(const KimaiTask &task) const;
        void    rebuild();

        KimaiTasks           m_tasks;
        QList<int>           m_shown; // indexes into m_tasks, in display order
        QString              m_filterText;
        int                  m_activeTimeSheetId = 0;
        std::optional<QDate> m_today;
    };
} // namespace kemai
