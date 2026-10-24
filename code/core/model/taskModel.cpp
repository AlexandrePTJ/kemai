// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "taskModel.h"

// STL headers
#include <algorithm>

// magic_enum headers
#include <magic_enum/magic_enum.hpp>

namespace
{
    int groupRank(const QString &group)
    {
        static const QStringList order{"progress", "overdue", "upcoming", "none"};
        return static_cast<int>(order.indexOf(group));
    }
} // namespace

namespace kemai
{
    TaskModel::TaskModel(QObject *parent): QAbstractListModel(parent) {}

    void TaskModel::setTasks(const KimaiTasks &tasks, int activeTimeSheetId)
    {
        beginResetModel();
        m_tasks             = tasks;
        m_activeTimeSheetId = activeTimeSheetId;
        rebuild();
        endResetModel();
        emit countChanged();
    }

    void TaskModel::setActiveTimeSheetId(int activeTimeSheetId)
    {
        if (activeTimeSheetId == m_activeTimeSheetId)
        {
            return;
        }
        setTasks(m_tasks, activeTimeSheetId);
    }

    void TaskModel::removeTask(int taskId)
    {
        KimaiTasks remaining;
        std::ranges::copy_if(m_tasks, std::back_inserter(remaining), [taskId](const KimaiTask &t) { return t.id != taskId; });
        setTasks(remaining, m_activeTimeSheetId);
    }

    const KimaiTask *TaskModel::findById(int taskId) const
    {
        const auto it = std::ranges::find_if(m_tasks, [taskId](const KimaiTask &t) { return t.id == taskId; });
        return it == m_tasks.cend() ? nullptr : &*it;
    }

    int TaskModel::totalCount() const
    {
        return static_cast<int>(m_tasks.size());
    }

    QString TaskModel::filterText() const
    {
        return m_filterText;
    }

    void TaskModel::setFilterText(const QString &text)
    {
        if (text == m_filterText)
        {
            return;
        }
        beginResetModel();
        m_filterText = text;
        rebuild();
        endResetModel();
        emit filterTextChanged();
        emit countChanged();
    }

    QString TaskModel::runningTaskTitle() const
    {
        const auto it = std::ranges::find_if(m_tasks, [this](const KimaiTask &t) { return isRunning(t); });
        return it == m_tasks.cend() ? QString() : it->title;
    }

    int TaskModel::groupCount(const QString &group) const
    {
        return static_cast<int>(std::ranges::count_if(m_shown, [&](int i)
                                                      { return (m_filterText.trimmed().isEmpty() ? groupOf(m_tasks[i]) : "matches") == group; }));
    }

    void TaskModel::setToday(const QDate &today)
    {
        m_today = today;
        setTasks(m_tasks, m_activeTimeSheetId);
    }

    bool TaskModel::isRunning(const KimaiTask &task) const
    {
        return m_activeTimeSheetId > 0 && std::ranges::any_of(task.activeTimeSheets, [this](const KimaiTimeSheet &ts) { return ts.id == m_activeTimeSheetId; });
    }

    QString TaskModel::dueStateOf(const KimaiTask &task) const
    {
        if (!task.endAt.isValid())
        {
            return "none";
        }
        const auto today = m_today.value_or(QDate::currentDate());
        const auto due   = task.endAt.toLocalTime().date();
        return due < today ? "overdue" : due == today ? "today" : "upcoming";
    }

    QString TaskModel::groupOf(const KimaiTask &task) const
    {
        if (task.status == KimaiTask::Status::Progress || isRunning(task))
        {
            return "progress";
        }
        const auto due = dueStateOf(task);
        return due == "today" ? "upcoming" : due;
    }

    void TaskModel::rebuild()
    {
        const auto query = m_filterText.trimmed();

        m_shown.clear();
        for (int i = 0; i < m_tasks.size(); ++i)
        {
            const auto &t = m_tasks[i];
            if (query.isEmpty() || t.title.contains(query, Qt::CaseInsensitive) || t.project.name.contains(query, Qt::CaseInsensitive)
                || t.activity.name.contains(query, Qt::CaseInsensitive) || t.todo.contains(query, Qt::CaseInsensitive))
            {
                m_shown.append(i);
            }
        }

        std::ranges::stable_sort(m_shown,
                                 [this](int a, int b)
                                 {
                                     const auto &ta = m_tasks[a];
                                     const auto &tb = m_tasks[b];
                                     const auto  ga = groupOf(ta);
                                     const auto  gb = groupOf(tb);
                                     if (ga != gb)
                                     {
                                         return groupRank(ga) < groupRank(gb);
                                     }
                                     if (ga == "progress" && isRunning(ta) != isRunning(tb))
                                     {
                                         return isRunning(ta);
                                     }
                                     if ((ga == "overdue" || ga == "upcoming") && ta.endAt != tb.endAt)
                                     {
                                         return ta.endAt < tb.endAt;
                                     }
                                     return ta.title.localeAwareCompare(tb.title) < 0;
                                 });
    }

    int TaskModel::rowCount(const QModelIndex &parent) const
    {
        return parent.isValid() ? 0 : static_cast<int>(m_shown.size());
    }

    QVariant TaskModel::data(const QModelIndex &index, int role) const
    {
        if (!index.isValid() || index.row() >= m_shown.size())
        {
            return {};
        }
        const auto &task = m_tasks[m_shown[index.row()]];

        switch (static_cast<Role>(role))
        {
        case Role::TaskId:
            return task.id;
        case Role::Title:
            return task.title;
        case Role::Todo:
            return task.todo;
        case Role::Status:
            return task.status == KimaiTask::Status::Progress ? QStringLiteral("progress") : QStringLiteral("pending");
        case Role::Running:
            return isRunning(task);
        case Role::ProjectId:
            return task.project.id;
        case Role::ProjectName:
            return task.project.name;
        case Role::ActivityId:
            return task.activity.id;
        case Role::ActivityName:
            return task.activity.name;
        case Role::ActivityColor:
            if (!task.activity.color.isEmpty())
            {
                return task.activity.color;
            }
            return task.project.color.isEmpty() ? QStringLiteral("#888888") : task.project.color;
        case Role::DueAt:
            return task.endAt.isValid() ? task.endAt.toLocalTime() : QDateTime();
        case Role::DueState:
            return dueStateOf(task);
        case Role::OverdueDays:
            return task.endAt.isValid() ? std::max<qint64>(0, task.endAt.toLocalTime().date().daysTo(m_today.value_or(QDate::currentDate()))) : 0;
        case Role::Estimation:
            return task.estimation;
        case Role::Group:
            return m_filterText.trimmed().isEmpty() ? groupOf(task) : QStringLiteral("matches");
        }
        return {};
    }

    QHash<int, QByteArray> TaskModel::roleNames() const
    {
        return {
            {magic_enum::enum_integer(Role::TaskId),        "taskId"       },
            {magic_enum::enum_integer(Role::Title),         "title"        },
            {magic_enum::enum_integer(Role::Todo),          "todo"         },
            {magic_enum::enum_integer(Role::Status),        "status"       },
            {magic_enum::enum_integer(Role::Running),       "running"      },
            {magic_enum::enum_integer(Role::ProjectId),     "projectId"    },
            {magic_enum::enum_integer(Role::ProjectName),   "projectName"  },
            {magic_enum::enum_integer(Role::ActivityId),    "activityId"   },
            {magic_enum::enum_integer(Role::ActivityName),  "activityName" },
            {magic_enum::enum_integer(Role::ActivityColor), "activityColor"},
            {magic_enum::enum_integer(Role::DueAt),         "dueAt"        },
            {magic_enum::enum_integer(Role::DueState),      "dueState"     },
            {magic_enum::enum_integer(Role::OverdueDays),   "overdueDays"  },
            {magic_enum::enum_integer(Role::Estimation),    "estimation"   },
            {magic_enum::enum_integer(Role::Group),         "group"        },
        };
    }
} // namespace kemai
