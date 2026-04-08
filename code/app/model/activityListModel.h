// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// Qt headers
#include <QAbstractListModel>
#include <QtQmlIntegration/qqmlintegration.h>

// Project headers
#include <client/kimaiResources.h>

namespace kemai
{
    class ActivityListModel : public QAbstractListModel
    {
        Q_OBJECT
        QML_ELEMENT
        QML_UNCREATABLE("ActivityListModel is created by the application")

    public:
        enum class Role
        {
            Label = Qt::UserRole + 1,
            ActivityName,
            ProjectName,
            ActivityColor,
            ActivityId,
        };

        explicit ActivityListModel(QObject *parent = nullptr);
        ~ActivityListModel() override;

        void setActivities(const KimaiActivities &activities);

        int                    rowCount(const QModelIndex &parent = {}) const override;
        QVariant               data(const QModelIndex &index, int role) const override;
        QHash<int, QByteArray> roleNames() const override;

    private:
        KimaiActivities m_activities;
    };
} // namespace kemai
