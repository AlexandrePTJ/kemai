// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// STL headers
#include <optional>

// Qt headers
#include <QJsonObject>
#include <QList>
#include <QJsonValue>
#include <QString>

namespace kemai
{
    struct KimaiCustomer
    {
        int     id      = 0;
        bool    visible = true;
        QString name;
        QString number;
        QString comment;
        QString company;
        QString address;
        QString countryKey;
        QString currencyKey;
        QString phone;
        QString fax;
        QString mobile;
        QString email;
        QString homepage;
        QString timezone;
        QString color;
        double  budget     = 0.0;
        int     timeBudget = 0; // seconds

        static KimaiCustomer fromJson(const QJsonValue &json);
        QJsonObject          toJson() const;
    };
    using KimaiCustomers = QList<KimaiCustomer>;

    struct KimaiProject
    {
        int           id      = 0;
        bool          visible = true;
        QString       name;
        KimaiCustomer customer;
        QString       comment;
        QString       orderNumber;
        QString       orderDate;
        QString       start;
        QString       end;
        QString       color;
        double        budget     = 0.0;
        int           timeBudget = 0;

        static KimaiProject fromJson(const QJsonValue &json);
        QJsonObject         toJson() const;
    };
    using KimaiProjects = QList<KimaiProject>;

    struct KimaiActivity
    {
        int                         id      = 0;
        bool                        visible = true;
        QString                     name;
        QString                     comment;
        QString                     color;
        double                      budget     = 0.0;
        int                         timeBudget = 0;
        std::optional<KimaiProject> project;

        static KimaiActivity fromJson(const QJsonValue &json);
        QJsonObject          toJson() const;
    };
    using KimaiActivities = QList<KimaiActivity>;

} // namespace kemai
