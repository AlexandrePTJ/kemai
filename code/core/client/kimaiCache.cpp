// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "kimaiCache.h"

// STL headers
#include <algorithm>
#include <utility>

// spdlog headers
#include <spdlog/spdlog.h>

// Project headers
#include "iKimaiClient.h"

namespace kemai
{
    KimaiCache::KimaiCache(QObject *parent):
    QObject(parent)
    {
    }

    void KimaiCache::load(IKimaiClient *client)
    {
        // A new generation supersedes any load in flight: its responses are dropped.
        const int generation = ++m_generation;

        m_staging          = {};
        m_stagingError.reset();
        m_pendingResources = {Resource::Customers, Resource::Projects, Resource::Activities, Resource::TimesheetConfig};

        client->requestCustomers()
            .then(this,
                  [this, generation](const KimaiCustomers &customers)
                  {
                      if (generation == m_generation)
                      {
                          m_staging.customers = customers;
                      }
                      onResourceLoaded(generation, Resource::Customers);
                  })
            .onFailed(this, [this, generation](const KimaiApiError &e)
                      { onResourceFailed(generation, Resource::Customers, e); });

        client->requestProjects()
            .then(this,
                  [this, generation](const KimaiProjects &projects)
                  {
                      if (generation == m_generation)
                      {
                          m_staging.projects = projects;
                      }
                      onResourceLoaded(generation, Resource::Projects);
                  })
            .onFailed(this, [this, generation](const KimaiApiError &e)
                      { onResourceFailed(generation, Resource::Projects, e); });

        client->requestActivities()
            .then(this,
                  [this, generation](const KimaiActivities &activities)
                  {
                      if (generation == m_generation)
                      {
                          m_staging.activities = activities;
                      }
                      onResourceLoaded(generation, Resource::Activities);
                  })
            .onFailed(this, [this, generation](const KimaiApiError &e)
                      { onResourceFailed(generation, Resource::Activities, e); });

        client->requestTimeSheetConfig()
            .then(this,
                  [this, generation](const KimaiTimeSheetConfig &config)
                  {
                      if (generation == m_generation)
                      {
                          m_staging.timesheetConfig = config;
                      }
                      onResourceLoaded(generation, Resource::TimesheetConfig);
                  })
            .onFailed(this, [this, generation](const KimaiApiError &e)
                      { onResourceFailed(generation, Resource::TimesheetConfig, e); });
    }

    void KimaiCache::onResourceLoaded(int generation, Resource resource)
    {
        if (generation != m_generation)
        {
            return;
        }

        m_pendingResources.remove(resource);
        if (!m_pendingResources.isEmpty())
        {
            return;
        }

        if (m_stagingError)
        {
            // Keep the previous complete snapshot (if any): never publish partial data.
            m_lastError = std::exchange(m_stagingError, std::nullopt);
            m_staging   = {};
            emit loadFailed();
            return;
        }

        consolidateProjects(m_staging);
        consolidateActivities(m_staging);

        m_current      = std::exchange(m_staging, {});
        m_lastLoadedAt = QDateTime::currentDateTimeUtc();
        m_lastError.reset();
        emit loaded();
    }

    void KimaiCache::onResourceFailed(int generation, Resource resource, const KimaiApiError &error)
    {
        if (generation != m_generation)
        {
            // Superseded load: its outcome no longer matters.
            spdlog::debug("KimaiCache: ignoring failure of superseded load: {}", error.what());
            return;
        }

        spdlog::error("KimaiCache: resource load failed: {}", error.what());
        if (!m_stagingError)
        {
            m_stagingError = error;
        }
        onResourceLoaded(generation, resource);
    }

    void KimaiCache::consolidateProjects(Snapshot &snapshot)
    {
        for (auto &project : snapshot.projects)
        {
            if (project.customer.id != 0)
            {
                const int  cid = project.customer.id;
                const auto it  = std::ranges::find_if(snapshot.customers, [cid](const KimaiCustomer &c)
                                                      { return c.id == cid; });
                if (it != snapshot.customers.cend())
                {
                    project.customer = *it;
                }
            }
        }
    }

    void KimaiCache::consolidateActivities(Snapshot &snapshot)
    {
        KimaiActivities consolidated;
        consolidated.reserve(snapshot.activities.size());

        for (const auto &activity : snapshot.activities)
        {
            if (activity.project.has_value())
            {
                const int  pid = activity.project->id;
                const auto it  = std::ranges::find_if(snapshot.projects, [pid](const KimaiProject &p)
                                                      { return p.id == pid; });

                KimaiActivity resolved = activity;
                if (it != snapshot.projects.cend())
                {
                    resolved.project = *it;
                }
                consolidated.append(std::move(resolved));
            }
            else
            {
                // Global activity (no project in the API payload): available for all
                // projects. Fan it out so each project gets its own "Project - Activity"
                // entry in the cache.
                for (const auto &project : snapshot.projects)
                {
                    KimaiActivity fanned = activity;
                    fanned.project       = project;
                    consolidated.append(std::move(fanned));
                }
            }
        }

        snapshot.activities = std::move(consolidated);
    }

    bool KimaiCache::isLoading() const
    {
        return !m_pendingResources.isEmpty();
    }

    bool KimaiCache::hasData() const
    {
        return m_lastLoadedAt.isValid();
    }

    QDateTime KimaiCache::lastLoadedAt() const
    {
        return m_lastLoadedAt;
    }

    const KimaiCustomers &KimaiCache::customers() const
    {
        return m_current.customers;
    }

    const KimaiProjects &KimaiCache::projects() const
    {
        return m_current.projects;
    }

    const KimaiActivities &KimaiCache::activities() const
    {
        return m_current.activities;
    }

    const KimaiTimeSheetConfig &KimaiCache::timesheetConfig() const
    {
        return m_current.timesheetConfig;
    }

    const std::optional<KimaiApiError> &KimaiCache::lastError() const
    {
        return m_lastError;
    }
} // namespace kemai
