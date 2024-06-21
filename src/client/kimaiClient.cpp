#include "kimaiClient.h"
#include "kimaiClient_p.h"

#include <QCoreApplication>
#include <QUrlQuery>

using namespace kemai;

/*
 * Static helpers
 */
static QByteArray toPostData(const QJsonValue& jsonValue)
{
    QJsonDocument jdoc;
    if (jsonValue.isArray())
    {
        jdoc = QJsonDocument(jsonValue.toArray());
    }
    else
    {
        jdoc = QJsonDocument(jsonValue.toObject());
    }
    return jdoc.toJson(QJsonDocument::Compact);
}

QString kemai::apiMethodToString(ApiMethod method)
{
    switch (method)
    {
    case ApiMethod::Version:
        return "version";

    case ApiMethod::Customers:
    case ApiMethod::CustomerAdd:
        return "customers";

    case ApiMethod::Projects:
    case ApiMethod::ProjectAdd:
        return "projects";

    case ApiMethod::Activities:
    case ApiMethod::ActivityAdd:
        return "activities";

    case ApiMethod::ActiveTimeSheets:
        return "timesheets/active";

    case ApiMethod::RecentTimeSheets:
        return "timesheets/recent";

    case ApiMethod::TimeSheets:
        return "timesheets";

    case ApiMethod::Users:
        return "users";

    case ApiMethod::MeUsers:
        return "users/me";

    case ApiMethod::Tags:
        return "tags";

    case ApiMethod::Tasks:
    case ApiMethod::TaskLog:
    case ApiMethod::TaskStart:
    case ApiMethod::TaskStop:
    case ApiMethod::TaskClose:
        return "tasks";

    case ApiMethod::Plugins:
        return "plugins";

    case ApiMethod::TimeSheetConfig:
        return "config/timesheet";

    default:
        return "";
    }
}

/*
 * Private impl
 */
KimaiClient::KimaiClientPrivate::KimaiClientPrivate(KimaiClient* c) : networkAccessManager(std::make_shared<QNetworkAccessManager>()), mQ(c) {}

QNetworkRequest KimaiClient::KimaiClientPrivate::prepareRequest(ApiMethod method, const std::map<QString, QString>& parameters, const QByteArray& data,
                                                                const QString& subPath) const
{
    /*
     * Create url
     */
    auto url = QUrl::fromUserInput(host);

    // Update existing path to work with custom path instances
    auto path = QString("%1/api/%2").arg(url.path(), apiMethodToString(method));
    if (!subPath.isEmpty())
    {
        path = QString("%1/%2").arg(path, subPath);
    }
    url.setPath(path);

    QUrlQuery query;
    for (const auto& [key, value] : parameters)
    {
        query.addQueryItem(key, value);
    }
    url.setQuery(query);

    /*
     * Create request
     */
    QNetworkRequest networkRequest;
    networkRequest.setUrl(url);

    // Until kimai 2.13, use username/password to identify. Use API Token from 2.14
    if (apiToken.isEmpty())
    {
        networkRequest.setRawHeader("X-AUTH-USER", username.toUtf8());
        networkRequest.setRawHeader("X-AUTH-TOKEN", token.toUtf8());
    }
    else
    {
        networkRequest.setRawHeader("Authorization", QString("Bearer %1").arg(apiToken).toLatin1());
    }
    networkRequest.setHeader(QNetworkRequest::UserAgentHeader, QString("%1/%2").arg(qApp->applicationName(), qApp->applicationVersion()));
    networkRequest.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    if (!data.isEmpty())
    {
        networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        networkRequest.setHeader(QNetworkRequest::ContentLengthHeader, data.size());
    }

    return networkRequest;
}

QNetworkReply* KimaiClient::KimaiClientPrivate::sendGetRequest(const QNetworkRequest& networkRequest) const
{
    spdlog::debug("[GET] {}", networkRequest.url().toString());
    return networkAccessManager->get(networkRequest);
}

QNetworkReply* KimaiClient::KimaiClientPrivate::sendPostRequest(const QNetworkRequest& networkRequest, const QByteArray& data) const
{
    spdlog::debug("[POST] {}", networkRequest.url().toString());
    return networkAccessManager->post(networkRequest, data);
}

QNetworkReply* KimaiClient::KimaiClientPrivate::sendPatchRequest(const QNetworkRequest& networkRequest, const QByteArray& data) const
{
    spdlog::debug("[PATCH] {}", networkRequest.url().toString());
    return networkAccessManager->sendCustomRequest(networkRequest, "PATCH", data);
}

/*
 * Public impl
 */
KimaiClient::KimaiClient(QObject* parent) : QObject(parent), mD(new KimaiClientPrivate(this)) {}

KimaiClient::~KimaiClient() = default;

void KimaiClient::setNetworkAccessManager(const std::shared_ptr<QNetworkAccessManager>& nam)
{
    mD->networkAccessManager = nam;
}

void KimaiClient::setHost(const QString& host)
{
    mD->host = host;
}

void KimaiClient::setLegacyAuth(const QString& username, const QString& token)
{
    mD->username = username;
    mD->token    = token;
}

bool KimaiClient::isUsingLegacyAuth() const
{
    return mD->apiToken.isEmpty();
}

void KimaiClient::setAPIToken(const QString& token)
{
    mD->apiToken = token;
}

VersionRequestResult KimaiClient::requestKimaiVersion()
{
    auto request = mD->prepareRequest(ApiMethod::Version);
    auto reply   = mD->sendGetRequest(request);
    return mD->processApiNetworkReplySingleObject<KimaiVersion>(ApiMethod::Version, reply);
}

MeRequestResult KimaiClient::requestMeUserInfo()
{
    auto request = mD->prepareRequest(ApiMethod::MeUsers);
    auto reply   = mD->sendGetRequest(request);
    return mD->processApiNetworkReplySingleObject<User>(ApiMethod::MeUsers, reply);
}

TimeSheetConfigResult KimaiClient::requestTimeSheetConfig()
{
    auto request = mD->prepareRequest(ApiMethod::TimeSheetConfig);
    auto reply   = mD->sendGetRequest(request);
    return mD->processApiNetworkReplySingleObject<TimeSheetConfig>(ApiMethod::TimeSheetConfig, reply);
}

PluginsResult KimaiClient::requestPlugins()
{
    auto request = mD->prepareRequest(ApiMethod::Plugins);
    auto reply   = mD->sendGetRequest(request);
    return mD->processApiNetworkReplyArray<Plugin>(ApiMethod::Plugins, reply);
}

CustomersResult KimaiClient::requestCustomers()
{
    auto request = mD->prepareRequest(ApiMethod::Customers);
    auto reply   = mD->sendGetRequest(request);
    return mD->processApiNetworkReplyArray<Customer>(ApiMethod::Customers, reply);
}

TimeSheetsResult KimaiClient::requestActiveTimeSheets()
{
    auto request = mD->prepareRequest(ApiMethod::ActiveTimeSheets);
    auto reply   = mD->sendGetRequest(request);
    return mD->processApiNetworkReplyArray<TimeSheet>(ApiMethod::ActiveTimeSheets, reply);
}

TimeSheetsResult KimaiClient::requestRecentTimeSheets()
{
    auto request = mD->prepareRequest(ApiMethod::RecentTimeSheets, std::map<QString, QString>{{"size", "5"}});
    auto reply   = mD->sendGetRequest(request);
    return mD->processApiNetworkReplyArray<TimeSheet>(ApiMethod::RecentTimeSheets, reply);
}

TimeSheetsResult KimaiClient::requestTimeSheets(const QString& term, int page)
{
    auto request = mD->prepareRequest(ApiMethod::TimeSheets, {{"page", QString::number(page)}, {"full", "1"}, {"term", term}});
    auto reply   = mD->sendGetRequest(request);
    return mD->processApiNetworkReplyArray<TimeSheet>(ApiMethod::TimeSheets, reply);
}

ProjectsResult KimaiClient::requestProjects(std::optional<int> customerId)
{
    std::map<QString, QString> parameters;
    if (customerId.has_value())
    {
        parameters.emplace("customer", QString::number(customerId.value()));
    }

    auto request = mD->prepareRequest(ApiMethod::Projects, parameters);
    auto reply   = mD->sendGetRequest(request);
    return mD->processApiNetworkReplyArray<Project>(ApiMethod::Projects, reply);
}

ActivitiesResult KimaiClient::requestActivities(std::optional<int> projectId)
{
    std::map<QString, QString> parameters;
    if (projectId.has_value())
    {
        parameters.emplace("project", QString::number(projectId.value()));
    }

    auto request = mD->prepareRequest(ApiMethod::Activities, parameters);
    auto reply   = mD->sendGetRequest(request);
    return mD->processApiNetworkReplyArray<Activity>(ApiMethod::Activities, reply);
}

CustomerAddResult KimaiClient::addCustomer(const Customer& customer)
{
    auto json    = KimaiApiTypesParser::toJson(customer);
    auto data    = toPostData(json);
    auto request = mD->prepareRequest(ApiMethod::CustomerAdd, {}, data);
    auto reply   = mD->sendPostRequest(request, data);
    return mD->processApiNetworkReplySingleObject<Customer>(ApiMethod::CustomerAdd, reply);
}

ProjectAddResult KimaiClient::addProject(const Project& project)
{
    auto json    = KimaiApiTypesParser::toJson(project);
    auto data    = toPostData(json);
    auto request = mD->prepareRequest(ApiMethod::ProjectAdd, {}, data);
    auto reply   = mD->sendPostRequest(request, data);
    return mD->processApiNetworkReplySingleObject<Project>(ApiMethod::ProjectAdd, reply);
}

ActivityAddResult KimaiClient::addActivity(const Activity& activity)
{
    auto json    = KimaiApiTypesParser::toJson(activity);
    auto data    = toPostData(json);
    auto request = mD->prepareRequest(ApiMethod::ActivityAdd, {}, data);
    auto reply   = mD->sendPostRequest(request, data);
    return mD->processApiNetworkReplySingleObject<Activity>(ApiMethod::ActivityAdd, reply);
}

TimeSheetResult KimaiClient::startTimeSheet(const TimeSheet& timeSheet, TimeSheetConfig::TrackingMode trackingMode)
{
    auto json    = KimaiApiTypesParser::toJson(timeSheet, trackingMode);
    auto data    = toPostData(json);
    auto request = mD->prepareRequest(ApiMethod::TimeSheets, {}, data);
    auto reply   = mD->sendPostRequest(request, data);
    return mD->processApiNetworkReplySingleObject<TimeSheet>(ApiMethod::TimeSheets, reply);
}

TimeSheetResult KimaiClient::updateTimeSheet(const TimeSheet& timeSheet, TimeSheetConfig::TrackingMode trackingMode)
{
    auto json    = KimaiApiTypesParser::toJson(timeSheet, trackingMode);
    auto data    = toPostData(json);
    auto request = mD->prepareRequest(ApiMethod::TimeSheets, {}, data, QString::number(timeSheet.id));
    auto reply   = mD->sendPatchRequest(request, data);
    return mD->processApiNetworkReplySingleObject<TimeSheet>(ApiMethod::TimeSheets, reply);
}

TasksResult KimaiClient::requestTasks()
{
    auto request = mD->prepareRequest(ApiMethod::Tasks);
    auto reply   = mD->sendGetRequest(request);
    return mD->processApiNetworkReplyArray<Task>(ApiMethod::Tasks, reply);
}

TaskResult KimaiClient::startTask(int taskId)
{
    auto request = mD->prepareRequest(ApiMethod::TaskStart, {}, {}, QString("%1/start").arg(taskId));
    auto reply   = mD->sendPatchRequest(request, {});
    return mD->processApiNetworkReplySingleObject<Task>(ApiMethod::TaskStart, reply);
}

TaskResult KimaiClient::closeTask(int taskId)
{
    auto request = mD->prepareRequest(ApiMethod::TaskClose, {}, {}, QString("%1/close").arg(taskId));
    auto reply   = mD->sendPatchRequest(request, {});
    return mD->processApiNetworkReplySingleObject<Task>(ApiMethod::TaskClose, reply);
}

void KimaiClient::addTrustedCertificates(const QStringList& trustedCertificates)
{
    auto sslConfiguration = QSslConfiguration::defaultConfiguration();
    for (const auto& pemStr : trustedCertificates)
    {
        auto certificates = sslConfiguration.caCertificates();
        certificates << QSslCertificate::fromData(pemStr.toLocal8Bit());
        sslConfiguration.setCaCertificates(certificates);
    }
    QSslConfiguration::setDefaultConfiguration(sslConfiguration);
}
