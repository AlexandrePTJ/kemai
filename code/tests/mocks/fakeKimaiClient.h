// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// STL headers
#include <memory>
#include <optional>

// Qt headers
#include <QCoreApplication>
#include <QFuture>
#include <QHash>
#include <QPromise>
#include <QStringList>
#include <QTimer>

// Project headers
#include <client/iKimaiClient.h>
#include <client/kimaiApiError.h>

namespace kemai
{
    /*!
     * IKimaiClient fake for unit tests: every request resolves immediately with
     * the configured payload, or fails with the KimaiApiError registered for the
     * method name. Calls are recorded in `calls`.
     */
    class FakeKimaiClient : public IKimaiClient
    {
    public:
        // Configurable payloads
        KimaiVersion         version;
        KimaiUser            user;
        KimaiTimeSheetConfig timesheetConfig;
        KimaiPlugins         plugins;
        KimaiCustomers       customers;
        KimaiProjects        projects;
        KimaiActivities      activities;
        KimaiTimeSheets      timeSheets;
        KimaiTimeSheets      activeTimeSheets;
        KimaiTasks           tasks;
        KimaiTimeSheet       timeSheetResult;
        QStringList          tags;
        KimaiTask            taskResult;

        // Method name -> error to fail with
        QHash<QString, KimaiApiError> errors;

        // Recorded method calls, in order
        QStringList calls;

        void failWith(const QString &method, ApiErrorKind kind, int httpStatus = 0)
        {
            errors.insert(method, KimaiApiError{kind, httpStatus, method, QStringLiteral("fake error")});
        }

        QFuture<KimaiVersion>         requestKimaiVersion() override { return result("requestKimaiVersion", version); }
        QFuture<KimaiUser>            requestMeUserInfo() override { return result("requestMeUserInfo", user); }
        QFuture<KimaiTimeSheetConfig> requestTimeSheetConfig() override { return result("requestTimeSheetConfig", timesheetConfig); }
        QFuture<KimaiPlugins>         requestPlugins() override { return result("requestPlugins", plugins); }
        QFuture<KimaiCustomers>       requestCustomers() override { return result("requestCustomers", customers); }
        QFuture<KimaiTimeSheets>      requestTimeSheets() override { return result("requestTimeSheets", timeSheets); }
        QFuture<KimaiTimeSheets>      requestActiveTimeSheets() override { return result("requestActiveTimeSheets", activeTimeSheets); }
        QFuture<KimaiTimeSheets>      requestRecentTimeSheets() override { return result("requestRecentTimeSheets", timeSheets); }
        QFuture<KimaiProjects>        requestProjects(std::optional<int> = std::nullopt) override { return result("requestProjects", projects); }
        QFuture<KimaiActivities>      requestActivities(std::optional<int> = std::nullopt) override { return result("requestActivities", activities); }
        QFuture<QStringList>          requestTags(const QString &name) override
        {
            lastTagsQuery = name;
            return result("requestTags", tags);
        }

        QFuture<KimaiCustomer> addCustomer(const KimaiCustomer &customer) override { return result("addCustomer", customer); }
        QFuture<KimaiProject>  addProject(const KimaiProject &project) override { return result("addProject", project); }
        QFuture<KimaiActivity> addActivity(const KimaiActivity &activity) override { return result("addActivity", activity); }

        QFuture<KimaiTimeSheet> startTimeSheet(const KimaiTimeSheet &, KimaiTimeSheetConfig::TrackingMode) override { return result("startTimeSheet", timeSheetResult); }
        QFuture<KimaiTimeSheet> updateTimeSheet(const KimaiTimeSheet &timeSheet, KimaiTimeSheetConfig::TrackingMode) override
        {
            lastUpdatedTimeSheet = timeSheet;
            return result("updateTimeSheet", timeSheetResult);
        }
        QFuture<void> deleteTimeSheet(int timeSheetId) override
        {
            lastDeletedTimeSheetId = timeSheetId;
            return result("deleteTimeSheet");
        }

        QFuture<KimaiTasks> requestTasks() override { return result("requestTasks", tasks); }
        QFuture<KimaiTask>  startTask(int) override { return result("startTask", taskResult); }
        QFuture<KimaiTask>  closeTask(int) override { return result("closeTask", taskResult); }

        KimaiTimeSheet lastUpdatedTimeSheet;
        int            lastDeletedTimeSheetId = 0;
        QString        lastTagsQuery;

    private:
        // Body-less variant (e.g. DELETE): resolves or fails like result()
        QFuture<void> result(const char *method)
        {
            const auto key = QString::fromLatin1(method);
            calls.append(key);

            std::optional<KimaiApiError> error;
            if (const auto it = errors.constFind(key); it != errors.constEnd())
            {
                error = it.value();
            }

            auto promise = std::make_shared<QPromise<void>>();
            promise->start();

            QTimer::singleShot(0, QCoreApplication::instance(),
                               [promise, error]()
                               {
                                   if (error)
                                   {
                                       promise->setException(std::make_exception_ptr(*error));
                                   }
                                   promise->finish();
                               });

            return promise->future();
        }

        // Resolves through the event loop (like the real network client) so that
        // continuations never run synchronously inside the caller.
        template<typename T>
        QFuture<T> result(const char *method, const T &value)
        {
            const auto key = QString::fromLatin1(method);
            calls.append(key);

            std::optional<KimaiApiError> error;
            if (const auto it = errors.constFind(key); it != errors.constEnd())
            {
                error = it.value();
            }

            auto promise = std::make_shared<QPromise<T>>();
            promise->start();

            QTimer::singleShot(0, QCoreApplication::instance(),
                               [promise, value, error]()
                               {
                                   if (error)
                                   {
                                       promise->setException(std::make_exception_ptr(*error));
                                   }
                                   else
                                   {
                                       promise->addResult(value);
                                   }
                                   promise->finish();
                               });

            return promise->future();
        }
    };
} // namespace kemai
