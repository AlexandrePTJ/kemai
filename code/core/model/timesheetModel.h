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
        // Shown rows (after the filter) and all listed entries
        Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
        Q_PROPERTY(int totalCount READ totalCount NOTIFY countChanged)
        // Keeps the entries whose project, activity, description or a tag contains it (case-insensitive)
        Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)

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
            CustomValues,
            Section, // "before": running since before the period, "period": the others, "" when no entry is "before"
            Planned  // starts after now
        };

        explicit TimesheetModel(QObject *parent = nullptr);
        ~TimesheetModel() override;

        // `periodStart`: first day of the history period (a running entry may have started before it)
        void setTimeSheets(const KimaiTimeSheets &timeSheets, const QDate &periodStart = {});
        // Adds the running entry when the list does not hold it (it started before the period)
        void ensureListed(const KimaiTimeSheet &running);
        // Project of the most recent entry among the listed and the week's ones, 0 without entries
        int latestProjectId() const;
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

        int     totalCount() const;
        QString filterText() const;
        void    setFilterText(const QString &text);

        QString todayDuration() const;
        QString weekDuration() const;
        QString dateLabel() const;

    signals:
        void aggregatesChanged();
        void countChanged();
        void filterTextChanged();

    private:
        QVariant roleValue(const KimaiTimeSheet &ts, TimesheetRole role) const;
        bool     startedBeforePeriod(const KimaiTimeSheet &ts) const;
        // Indexes of the entries matching the filter, in list order
        void rebuild();
        void computeAggregates();

        KimaiTimeSheets m_timeSheets;
        KimaiTimeSheets m_totalsTimeSheets;
        QDate           m_periodStart;
        QList<int>      m_shown;
        QString         m_filterText;
        bool            m_hasBeforePeriod = false;
        QString         m_todayDuration;
        QString         m_weekDuration;
        QString         m_dateLabel;
    };

} // namespace kemai
