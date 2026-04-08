// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// Qt headers
#include <QObject>
#include <QSet>

// Project headers
#include <client/kimaiResources.h>
#include <client/kimaiTimesheets.h>

namespace kemai
{
    class KimaiClient;

    class KimaiCache : public QObject
    {
        Q_OBJECT

    public:
        explicit KimaiCache(QObject *parent = nullptr);

        void load(KimaiClient *client);

        const KimaiCustomers       &customers() const;
        const KimaiProjects        &projects() const;
        const KimaiActivities      &activities() const;
        const KimaiTimeSheetConfig &timesheetConfig() const;

    signals:
        void loaded();

    private:
        enum class Resource
        {
            Customers,
            Projects,
            Activities,
            TimesheetConfig
        };

        void onResourceLoaded(Resource resource);
        void consolidateProjects();   // fills project.customer from m_customers
        void consolidateActivities(); // fills activity.project (with name) from m_projects

        KimaiCustomers       m_customers;
        KimaiProjects        m_projects;
        KimaiActivities      m_activities;
        KimaiTimeSheetConfig m_timesheetConfig;
        QSet<Resource>       m_pendingResources;
    };
} // namespace kemai
