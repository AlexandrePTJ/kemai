// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

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
            ProjectId
        };

        explicit TimesheetModel(QObject *parent = nullptr);
        ~TimesheetModel() override;

        void setTimeSheets(const KimaiTimeSheets &timeSheets);

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
        QString         m_todayDuration;
        QString         m_weekDuration;
        QString         m_dateLabel;
    };

} // namespace kemai
