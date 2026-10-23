// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// STL headers
#include <optional>

// Qt headers
#include <QObject>
#include <QSet>

// Project headers
#include <client/kimaiApiError.h>
#include <client/kimaiResources.h>
#include <client/kimaiTimesheets.h>

namespace kemai
{
    class IKimaiClient;

    class KimaiCache : public QObject
    {
        Q_OBJECT

    public:
        explicit KimaiCache(QObject *parent = nullptr);

        void load(IKimaiClient *client);

        const KimaiCustomers       &customers() const;
        const KimaiProjects        &projects() const;
        const KimaiActivities      &activities() const;
        const KimaiTimeSheetConfig &timesheetConfig() const;

        // Set when the last load() failed; an incomplete cache is useless, so any
        // resource failure fails the whole load.
        const std::optional<KimaiApiError> &lastError() const;

    signals:
        void loaded();
        void loadFailed();

    private:
        enum class Resource
        {
            Customers,
            Projects,
            Activities,
            TimesheetConfig
        };

        void onResourceLoaded(Resource resource);
        void onResourceFailed(Resource resource, const KimaiApiError &error);
        void consolidateProjects();   // fills project.customer from m_customers
        void consolidateActivities(); // fills activity.project (with name) from m_projects

        KimaiCustomers               m_customers;
        KimaiProjects                m_projects;
        KimaiActivities              m_activities;
        KimaiTimeSheetConfig         m_timesheetConfig;
        QSet<Resource>               m_pendingResources;
        std::optional<KimaiApiError> m_lastError;
    };
} // namespace kemai
