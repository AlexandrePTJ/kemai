// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// STL headers
#include <optional>

// Qt headers
#include <QFuture>
#include <QStringList>

// Project headers
#include <client/kimaiResources.h>
#include <client/kimaiSystem.h>
#include <client/kimaiTimesheets.h>

namespace kemai
{
    /*!
     * Abstract Kimai API client. Production code uses KimaiClient (network);
     * tests inject a fake resolving futures immediately.
     * Failed futures carry a KimaiApiError.
     */
    class IKimaiClient
    {
    public:
        virtual ~IKimaiClient() = default;

        virtual QFuture<KimaiVersion>         requestKimaiVersion()                                       = 0;
        virtual QFuture<KimaiUser>            requestMeUserInfo()                                         = 0;
        virtual QFuture<KimaiTimeSheetConfig> requestTimeSheetConfig()                                    = 0;
        virtual QFuture<KimaiPlugins>         requestPlugins()                                            = 0;
        virtual QFuture<KimaiCustomers>       requestCustomers()                                          = 0;
        virtual QFuture<KimaiTimeSheets>      requestTimeSheets()                                         = 0;
        virtual QFuture<KimaiTimeSheets>      requestActiveTimeSheets()                                   = 0;
        virtual QFuture<KimaiTimeSheets>      requestRecentTimeSheets()                                   = 0;
        virtual QFuture<KimaiProjects>        requestProjects(std::optional<int> customerId = std::nullopt) = 0;
        virtual QFuture<KimaiActivities>      requestActivities(std::optional<int> projectId = std::nullopt) = 0;
        virtual QFuture<QStringList>          requestTags(const QString &name)                             = 0; // names containing `name`

        virtual QFuture<KimaiCustomer> addCustomer(const KimaiCustomer &customer) = 0;
        virtual QFuture<KimaiProject>  addProject(const KimaiProject &project)    = 0;
        virtual QFuture<KimaiActivity> addActivity(const KimaiActivity &activity) = 0;
        // Kimai ignores unknown tag names sent with a timesheet: create them first (needs tag management rights)
        virtual QFuture<void>          createTag(const QString &name)             = 0;

        virtual QFuture<KimaiTimeSheet> startTimeSheet(const KimaiTimeSheet &timeSheet, KimaiTimeSheetConfig::TrackingMode trackingMode)  = 0;
        virtual QFuture<KimaiTimeSheet> updateTimeSheet(const KimaiTimeSheet &timeSheet, KimaiTimeSheetConfig::TrackingMode trackingMode) = 0;
        virtual QFuture<void>           deleteTimeSheet(int timeSheetId)                                                                 = 0;

        virtual QFuture<KimaiTasks> requestTasks()      = 0;
        virtual QFuture<KimaiTask>  startTask(int taskId) = 0;
        virtual QFuture<KimaiTask>  closeTask(int taskId) = 0;
    };
} // namespace kemai
