#include "kimaiCache.h"

/*
 * TODO: Removes when std::views is available on MacOS/clang
 */
#ifdef Q_OS_MACOS
#    include <range/v3/all.hpp>
#else
#    include <ranges>
namespace ranges = std;
#endif

#include <magic_enum.hpp>
#include <spdlog/spdlog.h>

using namespace kemai;

void KimaiCache::synchronize(const std::shared_ptr<KimaiClient>& client, const std::set<Category>& categories)
{
    if (!mSyncSemaphore.try_acquire())
    {
        spdlog::error("Sync already in progress");
        return;
    }

    mStatus = KimaiCache::Status::SyncPending;

    // Fill what to sync
    if (categories.empty())
    {
        auto categoryArray = magic_enum::enum_values<Category>();
        mPendingSync       = {categoryArray.begin(), categoryArray.end()};
    }
    else
    {
        mPendingSync = categories;
    }

    // Notify before running sync requests
    emit synchronizeStarted();

    for (const auto& category : mPendingSync)
    {
        switch (category)
        {
        case Category::Customers: {
            mCustomers.clear();
            auto customersResult = client->requestCustomers();
            connect(customersResult, &KimaiApiBaseResult::ready, this, [this, customersResult] { processCustomersResult(customersResult); });
            connect(customersResult, &KimaiApiBaseResult::error, this, [this, customersResult] { processCustomersResult(customersResult); });
        }
        break;

        case Category::Projects: {
            mProjects.clear();
            auto projectsResult = client->requestProjects();
            connect(projectsResult, &KimaiApiBaseResult::ready, this, [this, projectsResult] { processProjectsResult(projectsResult); });
            connect(projectsResult, &KimaiApiBaseResult::error, this, [this, projectsResult] { processProjectsResult(projectsResult); });
        }
        break;

        case Category::Activities: {
            mActivities.clear();
            auto activitiesResult = client->requestActivities();
            connect(activitiesResult, &KimaiApiBaseResult::ready, this, [this, activitiesResult] { processActivitiesResult(activitiesResult); });
            connect(activitiesResult, &KimaiApiBaseResult::error, this, [this, activitiesResult] { processActivitiesResult(activitiesResult); });
        }
        break;
        }
    }
}

KimaiCache::Status KimaiCache::status() const
{
    return mStatus;
}

Customers KimaiCache::customers() const
{
    return mCustomers;
}

Projects KimaiCache::projects(std::optional<int> customerId) const
{
    if (customerId.has_value())
    {
        auto its = mProjects | ranges::views::filter([id = customerId.value()](auto const& project) { return project.customer.id == id; });
        return {its.begin(), its.end()};
    }
    return mProjects;
}

Activities KimaiCache::activities(std::optional<int> projectId) const
{
    if (projectId.has_value())
    {
        auto its = mActivities | ranges::views::filter([id = projectId.value()](auto const& activity) {
                       if (!activity.project.has_value())
                       {
                           return true;
                       }
                       return activity.project->id == id;
                   });
        return {its.begin(), its.end()};
    }
    return mActivities;
}

void KimaiCache::updateSyncProgress(Category finishedCategory)
{
    const std::lock_guard<std::mutex> lockGuard(mProgressMutex);

    mPendingSync.erase(finishedCategory);

    if (mPendingSync.empty())
    {
        mStatus = KimaiCache::Status::Ready;
        mSyncSemaphore.release();
        emit synchronizeFinished();
    }
}

void KimaiCache::processCustomersResult(CustomersResult customersResult)
{
    if (!customersResult->hasError())
    {
        mCustomers = customersResult->takeResult();
    }
    customersResult->deleteLater();
    updateSyncProgress(Category::Customers);
}

void KimaiCache::processProjectsResult(ProjectsResult projectsResult)
{
    if (!projectsResult->hasError())
    {
        mProjects = projectsResult->takeResult();
    }
    projectsResult->deleteLater();
    updateSyncProgress(Category::Projects);
}

void KimaiCache::processActivitiesResult(ActivitiesResult activitiesResult)
{
    if (!activitiesResult->hasError())
    {
        mActivities = activitiesResult->takeResult();
    }
    activitiesResult->deleteLater();
    updateSyncProgress(Category::Activities);
}
