// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// STL headers
#include <optional>

// Qt headers
#include <QFuture>
#include <QObject>

// Project headers
#include <client/kimaiResources.h>
#include <client/kimaiSystem.h>
#include <client/kimaiTimesheets.h>

namespace kemai
{
    class KimaiClient : public QObject
    {
        Q_OBJECT

    public:
        explicit KimaiClient(QObject *parent = nullptr);
        ~KimaiClient() override;

        void setHost(const QString &host);
        void setToken(const QString &token);

        QFuture<KimaiVersion>         requestKimaiVersion();
        QFuture<KimaiUser>            requestMeUserInfo();
        QFuture<KimaiTimeSheetConfig> requestTimeSheetConfig();
        QFuture<KimaiPlugins>         requestPlugins();
        QFuture<KimaiCustomers>       requestCustomers();
        QFuture<KimaiTimeSheets>      requestActiveTimeSheets();
        QFuture<KimaiTimeSheets>      requestRecentTimeSheets();
        QFuture<KimaiProjects>        requestProjects(std::optional<int> customerId = std::nullopt);
        QFuture<KimaiActivities>      requestActivities(std::optional<int> projectId = std::nullopt);

        QFuture<KimaiCustomer>  addCustomer(const KimaiCustomer &customer);
        QFuture<KimaiProject>   addProject(const KimaiProject &project);
        QFuture<KimaiActivity>  addActivity(const KimaiActivity &activity);

        QFuture<KimaiTimeSheet> startTimeSheet(const KimaiTimeSheet &timeSheet, KimaiTimeSheetConfig::TrackingMode trackingMode);
        QFuture<KimaiTimeSheet> updateTimeSheet(const KimaiTimeSheet &timeSheet, KimaiTimeSheetConfig::TrackingMode trackingMode);

        QFuture<KimaiTasks> requestTasks();
        QFuture<KimaiTask>  startTask(int taskId);
        QFuture<KimaiTask>  closeTask(int taskId);

        static void addTrustedCertificates(const QStringList &trustedCertificates);

    signals:
        void sslError(const QString &msg, const QByteArray &certSN, const QByteArray &certPem);

    private:
        class KimaiClientPrivate;
        std::unique_ptr<KimaiClientPrivate> m_d;
    };
} // namespace kemai
