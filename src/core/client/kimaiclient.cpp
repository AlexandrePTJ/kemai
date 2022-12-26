#include "kimaiclient.h"
#include "kimaiclient_p.h"

#include "kemai_version.h"

#include <QCoreApplication>
#include <QUrlQuery>

#include <spdlog/spdlog.h>

using namespace kemai::client;

/*
 * Static helpers
 */
QByteArray toPostData(const QJsonValue& jsonValue)
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

QString kemai::client::apiMethodToString(ApiMethod method)
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
KimaiClient::KimaiClientPrivate::KimaiClientPrivate(KimaiClient* c) : networkAccessManager(new QNetworkAccessManager), mQ(c)
{
    connect(networkAccessManager.get(), &QNetworkAccessManager::sslErrors, this, &KimaiClientPrivate::onNamSslErrors);
}

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
    networkRequest.setRawHeader("X-AUTH-USER", username.toUtf8());
    networkRequest.setRawHeader("X-AUTH-TOKEN", token.toUtf8());
    networkRequest.setHeader(QNetworkRequest::UserAgentHeader, QString("%1/%2").arg(qApp->applicationName(), KEMAI_VERSION));
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
    spdlog::debug("===> [GET] {}", networkRequest.url().toString().toStdString());
    return networkAccessManager->get(networkRequest);
}

QNetworkReply* KimaiClient::KimaiClientPrivate::sendPostRequest(const QNetworkRequest& networkRequest, const QByteArray& data) const
{
    spdlog::debug("===> [POST] {}", networkRequest.url().toString().toStdString());
    return networkAccessManager->post(networkRequest, data);
}

QNetworkReply* KimaiClient::KimaiClientPrivate::sendPatchRequest(const QNetworkRequest& networkRequest, const QByteArray& data) const
{
    spdlog::debug("===> [PATCH] {}", networkRequest.url().toString().toStdString());
    return networkAccessManager->sendCustomRequest(networkRequest, "PATCH", data);
}

void KimaiClient::KimaiClientPrivate::onNamSslErrors(QNetworkReply* /*reply*/, const QList<QSslError>& errors)
{
    for (const auto& error : errors)
    {
        spdlog::error("<=== SSL Error: {}", error.errorString().toStdString());
    }

    // Process certificate errors one by one.
    const auto& crtError = errors.first();
    emit mQ->sslError(crtError.errorString(), crtError.certificate().serialNumber(), crtError.certificate().toPem());
}

/*
 * Public impl
 */
KimaiClient::KimaiClient(QObject* parent) : QObject(parent), mD(new KimaiClientPrivate(this)) {}

KimaiClient::~KimaiClient() = default;

void KimaiClient::setHost(const QString& host)
{
    mD->host = host;
}

void KimaiClient::setUsername(const QString& username)
{
    mD->username = username;
}

void KimaiClient::setToken(const QString& token)
{
    mD->token = token;
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

ProjectsResult KimaiClient::requestProjects(int customerId)
{
    auto request = mD->prepareRequest(ApiMethod::Projects, {{"customer", QString::number(customerId)}});
    auto reply   = mD->sendGetRequest(request);
    return mD->processApiNetworkReplyArray<Project>(ApiMethod::Projects, reply);
}

ActivitiesResult KimaiClient::requestActivities(int projectId)
{
    auto request = mD->prepareRequest(ApiMethod::Activities, {{"project", QString::number(projectId)}});
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

TimeSheetResult KimaiClient::stopTimeSheet(const TimeSheet& timeSheet)
{
    auto request = mD->prepareRequest(ApiMethod::TimeSheets, {}, {}, QString("%1/stop").arg(timeSheet.id));
    auto reply   = mD->sendPatchRequest(request, {});
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
