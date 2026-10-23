// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "kimaiCache.h"

// STL headers
#include <algorithm>

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
        m_pendingResources = {Resource::Customers, Resource::Projects, Resource::Activities, Resource::TimesheetConfig};
        m_lastError.reset();

        client->requestCustomers()
            .then(this,
                  [this](const KimaiCustomers &customers)
                  {
                      m_customers = customers;
                      onResourceLoaded(Resource::Customers);
                  })
            .onFailed(this, [this](const KimaiApiError &e)
                      { onResourceFailed(Resource::Customers, e); });

        client->requestProjects()
            .then(this,
                  [this](const KimaiProjects &projects)
                  {
                      m_projects = projects;
                      onResourceLoaded(Resource::Projects);
                  })
            .onFailed(this, [this](const KimaiApiError &e)
                      { onResourceFailed(Resource::Projects, e); });

        client->requestActivities()
            .then(this,
                  [this](const KimaiActivities &activities)
                  {
                      m_activities = activities;
                      onResourceLoaded(Resource::Activities);
                  })
            .onFailed(this, [this](const KimaiApiError &e)
                      { onResourceFailed(Resource::Activities, e); });

        client->requestTimeSheetConfig()
            .then(this,
                  [this](const KimaiTimeSheetConfig &config)
                  {
                      m_timesheetConfig = config;
                      onResourceLoaded(Resource::TimesheetConfig);
                  })
            .onFailed(this, [this](const KimaiApiError &e)
                      { onResourceFailed(Resource::TimesheetConfig, e); });
    }

    void KimaiCache::onResourceLoaded(Resource resource)
    {
        m_pendingResources.remove(resource);
        if (!m_pendingResources.isEmpty())
        {
            return;
        }

        if (m_lastError)
        {
            emit loadFailed();
            return;
        }

        consolidateProjects();
        consolidateActivities();
        emit loaded();
    }

    void KimaiCache::onResourceFailed(Resource resource, const KimaiApiError &error)
    {
        spdlog::error("KimaiCache: resource load failed: {}", error.what());
        if (!m_lastError)
        {
            m_lastError = error;
        }
        onResourceLoaded(resource);
    }

    void KimaiCache::consolidateProjects()
    {
        for (auto &project : m_projects)
        {
            if (project.customer.id != 0)
            {
                const int  cid = project.customer.id;
                const auto it  = std::ranges::find_if(m_customers, [cid](const KimaiCustomer &c)
                                                      { return c.id == cid; });
                if (it != m_customers.cend())
                {
                    project.customer = *it;
                }
            }
        }
    }

    void KimaiCache::consolidateActivities()
    {
        KimaiActivities consolidated;
        consolidated.reserve(m_activities.size());

        for (const auto &activity : m_activities)
        {
            if (activity.project.has_value())
            {
                const int  pid = activity.project->id;
                const auto it  = std::ranges::find_if(m_projects, [pid](const KimaiProject &p)
                                                      { return p.id == pid; });

                KimaiActivity resolved = activity;
                if (it != m_projects.cend())
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
                for (const auto &project : m_projects)
                {
                    KimaiActivity fanned = activity;
                    fanned.project       = project;
                    consolidated.append(std::move(fanned));
                }
            }
        }

        m_activities = std::move(consolidated);
    }

    const KimaiCustomers &KimaiCache::customers() const
    {
        return m_customers;
    }

    const KimaiProjects &KimaiCache::projects() const
    {
        return m_projects;
    }

    const KimaiActivities &KimaiCache::activities() const
    {
        return m_activities;
    }

    const KimaiTimeSheetConfig &KimaiCache::timesheetConfig() const
    {
        return m_timesheetConfig;
    }

    const std::optional<KimaiApiError> &KimaiCache::lastError() const
    {
        return m_lastError;
    }
} // namespace kemai
