#pragma once

#include <memory>
#include <optional>

#include <QNetworkAccessManager>
#include <QObject>

#include "client/kimaiAPI.h"
#include "client/kimaiReply.h"

namespace kemai {

using VersionRequestResult  = KimaiApiResult<KimaiVersion>*;
using MeRequestResult       = KimaiApiResult<User>*;
using TimeSheetConfigResult = KimaiApiResult<TimeSheetConfig>*;
using PluginsResult         = KimaiApiResult<Plugins>*;
using CustomersResult       = KimaiApiResult<Customers>*;
using TimeSheetsResult      = KimaiApiResult<TimeSheets>*;
using ProjectsResult        = KimaiApiResult<Projects>*;
using ActivitiesResult      = KimaiApiResult<Activities>*;
using CustomerAddResult     = KimaiApiResult<Customer>*;
using ProjectAddResult      = KimaiApiResult<Project>*;
using ActivityAddResult     = KimaiApiResult<Activity>*;
using TimeSheetResult       = KimaiApiResult<TimeSheet>*;
using TaskResult            = KimaiApiResult<Task>*;
using TasksResult           = KimaiApiResult<Tasks>*;

class KimaiClient : public QObject
{
    Q_OBJECT

public:
    explicit KimaiClient(QObject* parent = nullptr);
    ~KimaiClient() override;

    void setNetworkAccessManager(const std::shared_ptr<QNetworkAccessManager>& nam);

    void setHost(const QString& host);

    void setLegacyAuth(const QString& username, const QString& token);
    bool isUsingLegacyAuth() const;

    void setAPIToken(const QString& token);

    VersionRequestResult requestKimaiVersion() const;
    MeRequestResult requestMeUserInfo() const;
    TimeSheetConfigResult requestTimeSheetConfig() const;
    PluginsResult requestPlugins() const;
    CustomersResult requestCustomers() const;
    TimeSheetsResult requestActiveTimeSheets() const;
    TimeSheetsResult requestRecentTimeSheets() const;
    TimeSheetsResult requestTimeSheets(const QString& term = "", int page = 1) const;
    ProjectsResult requestProjects(std::optional<int> customerId = std::nullopt) const;
    ActivitiesResult requestActivities(std::optional<int> projectId = std::nullopt) const;

    CustomerAddResult addCustomer(const Customer& customer) const;
    ProjectAddResult addProject(const Project& project) const;
    ActivityAddResult addActivity(const Activity& activity) const;

    TimeSheetResult startTimeSheet(const TimeSheet& timeSheet, TimeSheetConfig::TrackingMode trackingMode) const;
    TimeSheetResult updateTimeSheet(const TimeSheet& timeSheet, TimeSheetConfig::TrackingMode trackingMode) const;

    TasksResult requestTasks() const;
    TaskResult startTask(int taskId) const;
    TaskResult closeTask(int taskId) const;

    static void addTrustedCertificates(const QStringList& trustedCertificates);

signals:
    void requestError(const QString& errorMsg);
    void sslError(const QString& msg, const QByteArray& certSN, const QByteArray& certPem);

private:
    class KimaiClientPrivate;
    QScopedPointer<KimaiClientPrivate> mD;
};

} // namespace kemai
