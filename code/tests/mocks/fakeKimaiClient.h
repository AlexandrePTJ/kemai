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
        KimaiMetaFieldRules  metaFieldRules;
        KimaiColorChoices    colorChoices;
        KimaiCustomers       customers;
        KimaiProjects        projects;
        KimaiActivities      activities;
        KimaiTimeSheets      timeSheets;
        KimaiTimeSheets      activeTimeSheets;
        // Reply to requestTimeSheets(since), `timeSheets` when unset
        std::optional<KimaiTimeSheets> weekTimeSheets;
        KimaiTasks                     tasks;
        KimaiTimeSheet                 timeSheetResult;
        QStringList                    tags;
        KimaiTask                      taskResult;

        // Method name -> error to fail with
        QHash<QString, KimaiApiError> errors;

        // Recorded method calls, in order
        QStringList calls;

        void failWith(const QString &method, ApiErrorKind kind, int httpStatus = 0)
        {
            errors.insert(method, KimaiApiError{kind, httpStatus, method, QStringLiteral("fake error")});
        }

        QFuture<KimaiVersion> requestKimaiVersion() override
        { return result("requestKimaiVersion", version); }
        QFuture<KimaiUser> requestMeUserInfo() override
        { return result("requestMeUserInfo", user); }
        QFuture<KimaiTimeSheetConfig> requestTimeSheetConfig() override
        { return result("requestTimeSheetConfig", timesheetConfig); }
        QFuture<KimaiPlugins> requestPlugins() override
        { return result("requestPlugins", plugins); }
        QFuture<KimaiMetaFieldRules> requestTimeSheetMetaFields() override
        { return result("requestTimeSheetMetaFields", metaFieldRules); }
        QFuture<KimaiColorChoices> requestColorChoices() override
        { return result("requestColorChoices", colorChoices); }
        QFuture<KimaiCustomers> requestCustomers() override
        { return result("requestCustomers", customers); }
        QFuture<KimaiTimeSheets> requestTimeSheets(std::optional<QDateTime> since = std::nullopt) override
        {
            if (since.has_value())
            {
                lastTimeSheetsSince = *since;
                return result("requestTimeSheetsSince", weekTimeSheets.value_or(timeSheets));
            }
            return result("requestTimeSheets", timeSheets);
        }
        QFuture<KimaiTimeSheets> requestActiveTimeSheets() override
        { return result("requestActiveTimeSheets", activeTimeSheets); }
        QFuture<KimaiTimeSheets> requestRecentTimeSheets() override
        { return result("requestRecentTimeSheets", timeSheets); }
        QFuture<KimaiProjects> requestProjects(std::optional<int> = std::nullopt) override
        { return result("requestProjects", projects); }
        QFuture<KimaiActivities> requestActivities(std::optional<int> = std::nullopt) override
        { return result("requestActivities", activities); }
        QFuture<QStringList> requestTags(const QString &name) override
        {
            lastTagsQuery = name;
            return result("requestTags", tags);
        }

        // Created resources echo what was sent, with the next id
        QFuture<KimaiCustomer> addCustomer(const KimaiCustomer &customer) override
        {
            lastAddedCustomer    = customer;
            lastAddedCustomer.id = nextCreatedId++;
            return result("addCustomer", lastAddedCustomer);
        }
        QFuture<KimaiProject> addProject(const KimaiProject &project) override
        {
            lastAddedProject    = project;
            lastAddedProject.id = nextCreatedId++;
            return result("addProject", lastAddedProject);
        }
        QFuture<KimaiActivity> addActivity(const KimaiActivity &activity) override
        {
            lastAddedActivity    = activity;
            lastAddedActivity.id = nextCreatedId++;
            return result("addActivity", lastAddedActivity);
        }
        QFuture<void> createTag(const QString &name) override
        {
            createdTags.append(name);
            return result("createTag");
        }

        QFuture<KimaiTimeSheet> startTimeSheet(const KimaiTimeSheet &, KimaiTimeSheetConfig::TrackingMode) override
        { return result("startTimeSheet", timeSheetResult); }
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

        // Records each write (and applies the successful ones to `timeSheets`); failures registered for "setTimeSheetMeta:<name>" affect that field only
        QFuture<KimaiTimeSheet> setTimeSheetMeta(int timeSheetId, const QString &name, const QString &value) override
        {
            metaWrites.append({timeSheetId, name, value});
            const auto perField = QStringLiteral("setTimeSheetMeta:%1").arg(name);
            if (errors.contains(perField))
            {
                calls.append(QStringLiteral("setTimeSheetMeta"));
                return result(qPrintable(perField), timeSheetResult);
            }
            // As the server: a written value shows in the next timesheet list
            if (!errors.contains(QStringLiteral("setTimeSheetMeta")))
            {
                for (auto &timeSheet : timeSheets)
                {
                    if (timeSheet.id == timeSheetId)
                    {
                        timeSheet.metaFields.insert(name, value);
                    }
                }
            }
            return result("setTimeSheetMeta", timeSheetResult);
        }

        QFuture<KimaiTimeSheet> stopTimeSheet(int timeSheetId) override
        {
            lastStoppedTimeSheetId = timeSheetId;
            return result("stopTimeSheet", timeSheetResult);
        }

        QFuture<KimaiTasks> requestTasks(int) override
        { return result("requestTasks", tasks); }
        QFuture<KimaiTimeSheet> startTask(int taskId) override
        {
            lastStartedTaskId = taskId;
            return result("startTask", timeSheetResult);
        }
        QFuture<KimaiTask> closeTask(int) override
        { return result("closeTask", taskResult); }

        struct MetaWrite
        {
            int     timeSheetId = 0;
            QString name;
            QString value;
        };
        QList<MetaWrite> metaWrites;

        KimaiTimeSheet lastUpdatedTimeSheet;
        int            lastDeletedTimeSheetId = 0;
        int            lastStoppedTimeSheetId = 0;
        int            lastStartedTaskId      = 0;
        KimaiCustomer  lastAddedCustomer;
        KimaiProject   lastAddedProject;
        KimaiActivity  lastAddedActivity;
        int            nextCreatedId = 100;
        QString        lastTagsQuery;
        QDateTime      lastTimeSheetsSince;
        QStringList    createdTags;

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

            QTimer::singleShot(0,
                               QCoreApplication::instance(),
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

            QTimer::singleShot(0,
                               QCoreApplication::instance(),
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
