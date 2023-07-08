#pragma once

#include <memory>
#include <optional>

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

    void setHost(const QString& host);
    void setUsername(const QString& username);
    void setToken(const QString& token);

    VersionRequestResult requestKimaiVersion();
    MeRequestResult requestMeUserInfo();
    TimeSheetConfigResult requestTimeSheetConfig();
    PluginsResult requestPlugins();
    CustomersResult requestCustomers();
    TimeSheetsResult requestActiveTimeSheets();
    TimeSheetsResult requestRecentTimeSheets();
    ProjectsResult requestProjects(std::optional<int> customerId = std::nullopt);
    ActivitiesResult requestActivities(std::optional<int> projectId = std::nullopt);

    CustomerAddResult addCustomer(const Customer& customer);
    ProjectAddResult addProject(const Project& project);
    ActivityAddResult addActivity(const Activity& activity);

    TimeSheetResult startTimeSheet(const TimeSheet& timeSheet, TimeSheetConfig::TrackingMode trackingMode);
    TimeSheetResult updateTimeSheet(const TimeSheet& timeSheet, TimeSheetConfig::TrackingMode trackingMode);

    TasksResult requestTasks();
    TaskResult startTask(int taskId);
    TaskResult closeTask(int taskId);

    static void addTrustedCertificates(const QStringList& trustedCertificates);

signals:
    void requestError(const QString& errorMsg);
    void sslError(const QString& msg, const QByteArray& certSN, const QByteArray& certPem);

private:
    class KimaiClientPrivate;
    QScopedPointer<KimaiClientPrivate> mD;
};

} // namespace kemai
