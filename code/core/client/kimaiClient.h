// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// STL headers
#include <optional>

// Qt headers
#include <QFuture>
#include <QJsonDocument>
#include <QObject>

// Project headers
#include <client/iKimaiClient.h>

namespace kemai
{
    class KimaiClient : public QObject, public IKimaiClient
    {
        Q_OBJECT

    public:
        explicit KimaiClient(QObject *parent = nullptr);
        ~KimaiClient() override;

        void setHost(const QString &host);
        void setToken(const QString &token);

        QFuture<KimaiVersion>         requestKimaiVersion() override;
        QFuture<KimaiUser>            requestMeUserInfo() override;
        QFuture<KimaiTimeSheetConfig> requestTimeSheetConfig() override;
        QFuture<KimaiPlugins>         requestPlugins() override;
        QFuture<KimaiCustomers>       requestCustomers() override;
        QFuture<KimaiTimeSheets>      requestTimeSheets() override;
        QFuture<KimaiTimeSheets>      requestActiveTimeSheets() override;
        QFuture<KimaiTimeSheets>      requestRecentTimeSheets() override;
        QFuture<KimaiProjects>        requestProjects(std::optional<int> customerId = std::nullopt) override;
        QFuture<KimaiActivities>      requestActivities(std::optional<int> projectId = std::nullopt) override;
        QFuture<QStringList>          requestTags(const QString &name) override;

        QFuture<KimaiCustomer> addCustomer(const KimaiCustomer &customer) override;
        QFuture<KimaiProject>  addProject(const KimaiProject &project) override;
        QFuture<KimaiActivity> addActivity(const KimaiActivity &activity) override;

        QFuture<KimaiTimeSheet> startTimeSheet(const KimaiTimeSheet &timeSheet, KimaiTimeSheetConfig::TrackingMode trackingMode) override;
        QFuture<KimaiTimeSheet> updateTimeSheet(const KimaiTimeSheet &timeSheet, KimaiTimeSheetConfig::TrackingMode trackingMode) override;
        QFuture<void>           deleteTimeSheet(int timeSheetId) override;

        QFuture<KimaiTasks> requestTasks() override;
        QFuture<KimaiTask>  startTask(int taskId) override;
        QFuture<KimaiTask>  closeTask(int taskId) override;

        static void addTrustedCertificates(const QStringList &trustedCertificates);

        // Tag names from a tags reply, accepting tag objects ({"name": ...}) or plain names; throws if not an array
        static QStringList tagNamesFromJson(const QJsonDocument &doc);

    signals:
        void sslError(const QString &msg, const QByteArray &certSN, const QByteArray &certPem);

    private:
        class KimaiClientPrivate;
        std::unique_ptr<KimaiClientPrivate> m_d;
    };
} // namespace kemai
