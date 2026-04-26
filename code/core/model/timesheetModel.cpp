// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "timesheetModel.h"

// magic_enum headers
#include <algorithm>
#include <magic_enum/magic_enum.hpp>

// Qt headers
#include <QDate>
#include <QDateTime>
#include <QLocale>

// Project headers
#include <misc/formatHelpers.h>

namespace
{

    QString formatTimeRange(const QDateTime &beginAt, const QDateTime &endAt)
    {
        const auto beginTime = beginAt.toLocalTime().toString(QStringLiteral("HH:mm"));

        if (!endAt.isValid())
        {
            return QStringLiteral("%1 \u2013 now").arg(beginTime);
        }

        const auto endTime = endAt.toLocalTime().toString(QStringLiteral("HH:mm"));
        return QStringLiteral("%1 \u2013 %2").arg(beginTime, endTime);
    }

} // namespace

namespace kemai
{
    TimesheetModel::TimesheetModel(QObject *parent):
    QAbstractListModel(parent),
    m_todayDuration(QStringLiteral("0h 00m")),
    m_weekDuration(QStringLiteral("0h 00m")),
    m_dateLabel(QLocale().toString(QDate::currentDate(), QStringLiteral("ddd, MMM d")))
    {
    }

    TimesheetModel::~TimesheetModel() = default;

    void TimesheetModel::setTimeSheets(const KimaiTimeSheets &timeSheets)
    {
        beginResetModel();

        m_timeSheets = timeSheets;

        // Show most recent timesheets first, with active timesheets (without endAt) on top
        std::ranges::sort(m_timeSheets, [](const KimaiTimeSheet &a, const KimaiTimeSheet &b)
                  { return a.beginAt > b.beginAt; });

        computeAggregates();

        endResetModel();
    }

    int TimesheetModel::rowCount(const QModelIndex &parent) const
    {
        return parent.isValid() ? 0 : static_cast<int>(m_timeSheets.size());
    }

    QVariant TimesheetModel::data(const QModelIndex &index, int role) const
    {
        if (!index.isValid() || index.row() >= m_timeSheets.size())
        {
            return {};
        }

        const auto &ts = m_timeSheets[index.row()];
        switch (static_cast<TimesheetRole>(role))
        {
        case TimesheetRole::Status:
            return ts.endAt.isValid() ? QStringLiteral("stopped") : QStringLiteral("active");

        case TimesheetRole::ActivityColor:
        {
            if (!ts.activity.color.isEmpty())
            {
                return ts.activity.color;
            }
            if (!ts.project.color.isEmpty())
            {
                return ts.project.color;
            }
            return QStringLiteral("#888888");
        }

        case TimesheetRole::ProjectName:
            return ts.project.name;

        case TimesheetRole::ActivityName:
            return ts.activity.name;

        case TimesheetRole::Description:
            return ts.description;

        case TimesheetRole::Duration:
        {
            const auto endTime = ts.endAt.isValid() ? ts.endAt : QDateTime::currentDateTimeUtc();
            return FormatHelpers::formatDuration(ts.beginAt.secsTo(endTime));
        }

        case TimesheetRole::TimeRange:
            return formatTimeRange(ts.beginAt, ts.endAt);

        case TimesheetRole::ActivityId:
            return ts.activity.id;

        case TimesheetRole::ProjectId:
            return ts.project.id;

        default:
            return {};
        }
    }

    QHash<int, QByteArray> TimesheetModel::roleNames() const
    {
        return {
            {magic_enum::enum_integer(TimesheetRole::Status),        "status"       },
            {magic_enum::enum_integer(TimesheetRole::ActivityColor), "activityColor"},
            {magic_enum::enum_integer(TimesheetRole::ProjectName),   "projectName"  },
            {magic_enum::enum_integer(TimesheetRole::ActivityName),  "activityName" },
            {magic_enum::enum_integer(TimesheetRole::Description),   "description"  },
            {magic_enum::enum_integer(TimesheetRole::Duration),      "duration"     },
            {magic_enum::enum_integer(TimesheetRole::TimeRange),     "timeRange"    },
            {magic_enum::enum_integer(TimesheetRole::ActivityId),    "activityId"   },
            {magic_enum::enum_integer(TimesheetRole::ProjectId),     "projectId"    }
        };
    }

    void TimesheetModel::refreshActiveDurations()
    {
        for (int i = 0; i < m_timeSheets.size(); ++i)
        {
            if (!m_timeSheets[i].endAt.isValid())
            {
                const auto idx = index(i);
                emit       dataChanged(idx, idx, {magic_enum::enum_integer(TimesheetRole::Duration), magic_enum::enum_integer(TimesheetRole::TimeRange)});
            }
        }
        computeAggregates();
    }

    QString TimesheetModel::todayDuration() const
    {
        return m_todayDuration;
    }

    QString TimesheetModel::weekDuration() const
    {
        return m_weekDuration;
    }

    QString TimesheetModel::dateLabel() const
    {
        return m_dateLabel;
    }

    void TimesheetModel::computeAggregates()
    {
        const auto now            = QDateTime::currentDateTimeUtc();
        const auto todayLocal     = QDate::currentDate();
        const int  dayOfWeek      = todayLocal.dayOfWeek(); // 1=Monday, 7=Sunday
        const auto weekStartLocal = todayLocal.addDays(-(dayOfWeek - 1));

        qint64 todaySeconds = 0;
        qint64 weekSeconds  = 0;

        for (const auto &ts : m_timeSheets)
        {
            const auto endTime   = ts.endAt.isValid() ? ts.endAt : now;
            const auto beginDate = ts.beginAt.toLocalTime().date();
            const auto duration  = ts.beginAt.secsTo(endTime);

            if (beginDate == todayLocal)
            {
                todaySeconds += duration;
            }

            if (beginDate >= weekStartLocal && beginDate <= todayLocal)
            {
                weekSeconds += duration;
            }
        }

        const auto newToday = FormatHelpers::formatDurationShort(todaySeconds);
        const auto newWeek  = FormatHelpers::formatDurationShort(weekSeconds);
        const auto newLabel = QLocale().toString(todayLocal, QStringLiteral("ddd, MMM d"));

        if (m_todayDuration != newToday || m_weekDuration != newWeek || m_dateLabel != newLabel)
        {
            m_todayDuration = newToday;
            m_weekDuration  = newWeek;
            m_dateLabel     = newLabel;
            emit aggregatesChanged();
        }
    }

} // namespace kemai
