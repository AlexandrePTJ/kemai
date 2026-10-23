// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "activityListModel.h"

// magic_enum headers
#include <magic_enum/magic_enum.hpp>

namespace kemai
{
    ActivityListModel::ActivityListModel(QObject *parent):
    QAbstractListModel(parent)
    {
    }

    ActivityListModel::~ActivityListModel() = default;

    void ActivityListModel::setActivities(const KimaiActivities &activities)
    {
        beginResetModel();
        m_activities = activities;
        endResetModel();
    }

    int ActivityListModel::rowCount(const QModelIndex &parent) const
    {
        return parent.isValid() ? 0 : static_cast<int>(m_activities.size());
    }

    QVariant ActivityListModel::data(const QModelIndex &index, int role) const
    {
        if (!index.isValid() || index.row() >= m_activities.size())
        {
            return {};
        }

        const auto &activity = m_activities[index.row()];
        switch (static_cast<Role>(role))
        {
        case Role::Label:
            return activity.project.has_value()
                ? QStringLiteral("%1 - %2").arg(activity.project->name, activity.name)
                : activity.name;

        case Role::ActivityName:
            return activity.name;

        case Role::ProjectName:
            return activity.project.has_value() ? activity.project->name : QString{};

        case Role::ActivityColor:
            if (!activity.color.isEmpty())
            {
                return activity.color;
            }
            if (activity.project.has_value() && !activity.project->color.isEmpty())
            {
                return activity.project->color;
            }
            return QStringLiteral("#888888");

        case Role::ActivityId:
            return activity.id;

        case Role::ProjectId:
            return activity.project.has_value() ? activity.project->id : 0;
        }

        return {};
    }

    QHash<int, QByteArray> ActivityListModel::roleNames() const
    {
        return {
            {magic_enum::enum_integer(Role::Label),         "label"        },
            {magic_enum::enum_integer(Role::ActivityName),  "activityName" },
            {magic_enum::enum_integer(Role::ProjectName),   "projectName"  },
            {magic_enum::enum_integer(Role::ActivityColor), "activityColor"},
            {magic_enum::enum_integer(Role::ActivityId),    "activityId"   },
            {magic_enum::enum_integer(Role::ProjectId),     "projectId"    },
        };
    }
} // namespace kemai
