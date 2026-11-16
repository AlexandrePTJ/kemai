// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// Qt headers
#include <QSortFilterProxyModel>
#include <QString>
#include <QtQmlIntegration/qqmlintegration.h>

namespace kemai
{
    // Log journal entries at or above a minimum level (LogEntryModel::Level as int)
    class LogFilterProxyModel : public QSortFilterProxyModel
    {
        Q_OBJECT
        QML_ELEMENT

        Q_PROPERTY(int minLevel READ minLevel WRITE setMinLevel NOTIFY minLevelChanged)
        Q_PROPERTY(int count READ count NOTIFY countChanged)

    public:
        explicit LogFilterProxyModel(QObject *parent = nullptr);
        ~LogFilterProxyModel() override;

        int  minLevel() const;
        void setMinLevel(int level);
        int  count() const;

        // Listed rows `first` to `last` as text lines (LogEntryModel::textLine), one per line
        Q_INVOKABLE QString text(int first, int last) const;

    signals:
        void minLevelChanged();
        void countChanged();

    protected:
        bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    private:
        int m_minLevel = 0;
    };
} // namespace kemai
