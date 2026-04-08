// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "activityFilterProxyModel.h"

// Qt headers
#include <QAbstractItemModel>
#include <QStringList>

// magic_enum headers
#include <magic_enum/magic_enum.hpp>

// Project headers
#include "activityListModel.h"
#include <misc/fuzzyMatch.h>

namespace kemai
{
    ActivityFilterProxyModel::ActivityFilterProxyModel(QObject *parent):
    QSortFilterProxyModel(parent)
    {
        setDynamicSortFilter(true);
    }

    ActivityFilterProxyModel::~ActivityFilterProxyModel() = default;

    QString ActivityFilterProxyModel::filterText() const
    {
        return m_filterText;
    }

    void ActivityFilterProxyModel::setFilterText(const QString &text)
    {
        if (m_filterText == text)
        {
            return;
        }
        beginFilterChange();
        m_filterText = text;
        endFilterChange(QSortFilterProxyModel::Direction::Rows);
        emit filterTextChanged();
    }

    bool ActivityFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
    {
        if (m_filterText.isEmpty())
        {
            return true;
        }

        const auto *src = sourceModel();
        if (src == nullptr)
        {
            return true;
        }

        const auto idx   = src->index(sourceRow, 0, sourceParent);
        const auto label = src->data(idx, magic_enum::enum_integer(ActivityListModel::Role::Label)).toString();
        return FuzzyMatch::wordPrefixMatch(m_filterText, label);
    }
} // namespace kemai
