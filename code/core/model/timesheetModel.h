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
    class TimesheetModel : public QAbstractListModel
    {
        Q_OBJECT

        Q_PROPERTY(QString todayDuration READ todayDuration NOTIFY aggregatesChanged)
        Q_PROPERTY(QString weekDuration READ weekDuration NOTIFY aggregatesChanged)
        Q_PROPERTY(QString dateLabel READ dateLabel NOTIFY aggregatesChanged)

    public:
        enum class TimesheetRole
        {
            Status = Qt::UserRole + 1,
            ActivityColor,
            ProjectName,
            ActivityName,
            Description,
            Duration,
            TimeRange,
            ActivityId,
            ProjectId,
            TimesheetId,
            BeginAt,
            EndAt,
            Tags,
            CustomValues
        };

        explicit TimesheetModel(QObject *parent = nullptr);
        ~TimesheetModel() override;

        void setTimeSheets(const KimaiTimeSheets &timeSheets);
        // Entries the totals are computed from: the list holds only the latest ones, which may not cover the week
        void setTotalsTimeSheets(const KimaiTimeSheets &timeSheets);
        // What setTotalsTimeSheets must cover: the week, a day earlier for a server in another time zone
        static QDateTime totalsPeriodStart();

        std::optional<KimaiTimeSheet> findById(int timesheetId) const;
        // The entry's roles by name (as a delegate sees them), empty when it is not in the list
        Q_INVOKABLE QVariantMap entry(int timesheetId) const;

        int                    rowCount(const QModelIndex &parent = {}) const override;
        QVariant               data(const QModelIndex &index, int role) const override;
        QHash<int, QByteArray> roleNames() const override;

        void refreshActiveDurations();

        QString todayDuration() const;
        QString weekDuration() const;
        QString dateLabel() const;

    signals:
        void aggregatesChanged();

    private:
        void computeAggregates();

        KimaiTimeSheets m_timeSheets;
        KimaiTimeSheets m_totalsTimeSheets;
        QString         m_todayDuration;
        QString         m_weekDuration;
        QString         m_dateLabel;
    };

} // namespace kemai
