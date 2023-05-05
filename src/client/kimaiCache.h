#pragma once

#include <memory>
#include <mutex>
#include <set>

#include <QObject>

#include "client/kimaiclient.h"

namespace kemai {

class KimaiCache : public QObject
{
    Q_OBJECT

public:
    enum class Category
    {
        Customers,
        Projects,
        Activities
    };

    enum class Status
    {
        Empty,
        SyncPending,
        Ready
    };

    void synchronize(const std::shared_ptr<KimaiClient>& client, const std::set<Category>& categories = {});
    Status status() const;

    Customers customers() const;
    Projects projects(std::optional<int> customerId = std::nullopt) const;
    Activities activities(std::optional<int> projectId = std::nullopt) const;

signals:
    void synchronizeStarted();
    void synchronizeFinished();

private:
    void updateSyncProgress(Category finishedCategory);
    void processCustomersResult(CustomersResult customersResult);
    void processProjectsResult(ProjectsResult projectsResult);
    void processActivitiesResult(ActivitiesResult activitiesResult);

    std::set<Category> mPendingSync;

    Customers mCustomers;
    Projects mProjects;
    Activities mActivities;

    kemai::KimaiCache::Status mStatus = kemai::KimaiCache::Status::Empty;
    std::mutex mSyncMutex;
    std::mutex mProgressMutex;
};

} // namespace kemai
