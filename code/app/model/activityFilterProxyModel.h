// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// Qt headers
#include <QSortFilterProxyModel>
#include <QString>
#include <QtQmlIntegration/qqmlintegration.h>

namespace kemai
{
    class ActivityFilterProxyModel : public QSortFilterProxyModel
    {
        Q_OBJECT
        QML_ELEMENT

        Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)

    public:
        explicit ActivityFilterProxyModel(QObject *parent = nullptr);
        ~ActivityFilterProxyModel() override;

        QString filterText() const;
        void    setFilterText(const QString &text);

    signals:
        void filterTextChanged();

    protected:
        bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    private:
        QString m_filterText;
    };
} // namespace kemai
