#pragma once

#include <memory>
#include <mutex>
#include <semaphore>
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

    void synchronize(const std::shared_ptr<KimaiClient>& client, const std::set<Category>& categories = {});

    Customers customers() const;
    Projects projects(std::optional<int> customerId) const;
    Activities activities(std::optional<int> projectId) const;

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

    std::binary_semaphore mSyncSemaphore{1};
    std::mutex mProgressMutex;
};

} // namespace kemai
