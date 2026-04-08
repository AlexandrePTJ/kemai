// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "kimaiCache.h"

// STL headers
#include <algorithm>

// spdlog headers
#include <spdlog/spdlog.h>

// Project headers
#include "kimaiClient.h"

namespace kemai
{
    KimaiCache::KimaiCache(QObject *parent):
    QObject(parent)
    {
    }

    void KimaiCache::load(KimaiClient *client)
    {
        m_pendingResources = {Resource::Customers, Resource::Projects, Resource::Activities, Resource::TimesheetConfig};

        client->requestCustomers()
            .then(this,
                  [this](const KimaiCustomers &customers)
                  {
                      m_customers = customers;
                      onResourceLoaded(Resource::Customers);
                  })
            .onFailed(this,
                      [this](const std::exception &e)
                      {
                          spdlog::error("KimaiCache: failed to fetch customers: {}", e.what());
                          onResourceLoaded(Resource::Customers);
                      });

        client->requestProjects()
            .then(this,
                  [this](const KimaiProjects &projects)
                  {
                      m_projects = projects;
                      onResourceLoaded(Resource::Projects);
                  })
            .onFailed(this,
                      [this](const std::exception &e)
                      {
                          spdlog::error("KimaiCache: failed to fetch projects: {}", e.what());
                          onResourceLoaded(Resource::Projects);
                      });

        client->requestActivities()
            .then(this,
                  [this](const KimaiActivities &activities)
                  {
                      m_activities = activities;
                      onResourceLoaded(Resource::Activities);
                  })
            .onFailed(this,
                      [this](const std::exception &e)
                      {
                          spdlog::error("KimaiCache: failed to fetch activities: {}", e.what());
                          onResourceLoaded(Resource::Activities);
                      });

        client->requestTimeSheetConfig()
            .then(this,
                  [this](const KimaiTimeSheetConfig &config)
                  {
                      m_timesheetConfig = config;
                      onResourceLoaded(Resource::TimesheetConfig);
                  })
            .onFailed(this,
                      [this](const std::exception &e)
                      {
                          spdlog::error("KimaiCache: failed to fetch timesheet config: {}", e.what());
                          onResourceLoaded(Resource::TimesheetConfig);
                      });
    }

    void KimaiCache::onResourceLoaded(Resource resource)
    {
        m_pendingResources.remove(resource);
        if (m_pendingResources.isEmpty())
        {
            consolidateProjects();
            consolidateActivities();
            emit loaded();
        }
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
        for (auto &activity : m_activities)
        {
            if (activity.project.has_value())
            {
                const int  pid = activity.project->id;
                const auto it  = std::ranges::find_if(m_projects, [pid](const KimaiProject &p)
                                                      { return p.id == pid; });
                if (it != m_projects.cend())
                {
                    activity.project = *it;
                }
            }
        }
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
} // namespace kemai
