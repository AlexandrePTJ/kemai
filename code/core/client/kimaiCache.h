// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// STL headers
#include <optional>

// Qt headers
#include <QDateTime>
#include <QObject>
#include <QSet>

// Project headers
#include <client/kimaiApiError.h>
#include <client/kimaiResources.h>
#include <client/kimaiTimesheets.h>

namespace kemai
{
    class IKimaiClient;

    /*!
     * Customers/projects/activities/timesheet config, loaded all-or-nothing.
     * A load fills a staging snapshot that is swapped in only once every resource
     * succeeded: a failed (re)load never publishes partial data and keeps the
     * previous complete snapshot. Calling load() while one is in flight supersedes
     * it; late responses of the older load are ignored.
     */
    class KimaiCache : public QObject
    {
        Q_OBJECT

    public:
        explicit KimaiCache(QObject *parent = nullptr);

        void load(IKimaiClient *client);

        bool      isLoading() const;
        bool      hasData() const;      // true once a load succeeded
        QDateTime lastLoadedAt() const; // UTC, invalid until a load succeeded

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

        struct Snapshot
        {
            KimaiCustomers       customers;
            KimaiProjects        projects;
            KimaiActivities      activities;
            KimaiTimeSheetConfig timesheetConfig;
        };

        void onResourceLoaded(int generation, Resource resource);
        void onResourceFailed(int generation, Resource resource, const KimaiApiError &error);

        static void consolidateProjects(Snapshot &snapshot);   // fills project.customer from customers
        static void consolidateActivities(Snapshot &snapshot); // fills activity.project (with name) from projects

        Snapshot                     m_current;
        Snapshot                     m_staging;
        QSet<Resource>               m_pendingResources;
        std::optional<KimaiApiError> m_stagingError;
        std::optional<KimaiApiError> m_lastError;
        QDateTime                    m_lastLoadedAt;
        int                          m_generation = 0;
    };
} // namespace kemai
