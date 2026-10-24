// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// STL headers
#include <chrono>
#include <optional>

// Qt headers
#include <QFuture>
#include <QJsonDocument>
#include <QObject>

// Project headers
#include <client/iKimaiClient.h>
#include <client/kimaiCertificate.h>

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

        // A request that transfers nothing for this long fails as NoNetwork: a dead connection never errors on its own
        static constexpr std::chrono::milliseconds DefaultTransferTimeout{15000};
        void setTransferTimeout(std::chrono::milliseconds timeout);
        // Certificates the user trusted, each for its own host: their "unknown issuer" errors are ignored
        void setTrustedCertificates(const TrustedCertificates &certificates);

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
        QFuture<KimaiTimeSheet> stopTimeSheet(int timeSheetId) override;
        QFuture<void>           createTag(const QString &name) override;

        QFuture<KimaiTasks> requestTasks(int userId) override;
        QFuture<KimaiTimeSheet> startTask(int taskId) override;
        QFuture<KimaiTask>  closeTask(int taskId) override;

        static void addTrustedCertificates(const QStringList &trustedCertificates);

        // Tag names from a tags reply, accepting tag objects ({"name": ...}) or plain names; throws if not an array
        static QStringList tagNamesFromJson(const QJsonDocument &doc);
        // Host as typed by the user → base URL: https:// when no scheme is given (QUrl::fromUserInput would pick
        // http://, and servers redirecting to https turn PATCH/POST into GET: 405), no trailing slash
        static QString normalizedHost(const QString &host);
        // Kimai's 400 body ({"errors": {"children": {field: {"errors": [...]}}}}): the first field with an error and its
        // message, or the form-level error with an empty field; empty pair when the body holds none
        static std::pair<QString, QString> validationErrorFromJson(const QJsonDocument &doc);


    private:
        class KimaiClientPrivate;
        std::unique_ptr<KimaiClientPrivate> m_d;
    };
} // namespace kemai
