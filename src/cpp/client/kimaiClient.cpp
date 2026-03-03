// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "kimaiClient.h"
#include "kimaiClient_p.h"

// 3rd party headers
#include <spdlog/spdlog.h>

// Qt headers
#include <QCoreApplication>
#include <QFuture>
#include <QUrlQuery>

// Project headers
#include <misc/jsonHelpers.h>

using namespace kemai;

/*
 * Static helpers
 */
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
KimaiClient::KimaiClientPrivate::KimaiClientPrivate(KimaiClient *c):
networkAccessManager(std::make_unique<QNetworkAccessManager>()),
m_q(c)
{
    connect(networkAccessManager.get(), &QNetworkAccessManager::sslErrors, this, &KimaiClientPrivate::onNamSslErrors);
}

QNetworkRequest KimaiClient::KimaiClientPrivate::prepareRequest(ApiMethod method, const std::map<QString, QString> &parameters, const QByteArray &data, const QString &subPath) const
{
    auto url  = QUrl::fromUserInput(host);
    auto path = QString("%1/api/%2").arg(url.path(), apiMethodToString(method));
    if (!subPath.isEmpty())
    {
        path = QString("%1/%2").arg(path, subPath);
    }
    url.setPath(path);

    QUrlQuery query;
    for (const auto &[key, value] : parameters)
    {
        query.addQueryItem(key, value);
    }
    url.setQuery(query);

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
    networkRequest.setRawHeader("Accept", "application/json");

    if (!data.isEmpty())
    {
        networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        networkRequest.setHeader(QNetworkRequest::ContentLengthHeader, data.size());
    }

    return networkRequest;
}

QNetworkReply *KimaiClient::KimaiClientPrivate::sendGetRequest(const QNetworkRequest &networkRequest) const
{
    spdlog::debug("[GET] {}", networkRequest.url().toString());
    return networkAccessManager->get(networkRequest);
}

QNetworkReply *KimaiClient::KimaiClientPrivate::sendPostRequest(const QNetworkRequest &networkRequest, const QByteArray &data) const
{
    spdlog::debug("[POST] {}", networkRequest.url().toString());
    return networkAccessManager->post(networkRequest, data);
}

QNetworkReply *KimaiClient::KimaiClientPrivate::sendPatchRequest(const QNetworkRequest &networkRequest, const QByteArray &data) const
{
    spdlog::debug("[PATCH] {}", networkRequest.url().toString());
    return networkAccessManager->sendCustomRequest(networkRequest, "PATCH", data);
}

void KimaiClient::KimaiClientPrivate::onNamSslErrors(QNetworkReply * /*reply*/, const QList<QSslError> &errors)
{
    for (const auto &error : errors)
    {
        spdlog::error("SSL Error: {}", error.errorString());
    }

    const auto &crtError = errors.first();
    emit        m_q->sslError(crtError.errorString(), crtError.certificate().serialNumber(), crtError.certificate().toPem());
}

/*
 * Public impl
 */
KimaiClient::KimaiClient(QObject *parent):
QObject(parent),
m_d(std::make_unique<KimaiClientPrivate>(this))
{
}

KimaiClient::~KimaiClient() = default;

void KimaiClient::setHost(const QString &host)
{
    m_d->host = host;
}

void KimaiClient::setToken(const QString &token)
{
    m_d->apiToken = token;
}

QFuture<KimaiVersion> KimaiClient::requestKimaiVersion()
{
    auto request = m_d->prepareRequest(ApiMethod::Version);
    auto reply   = m_d->sendGetRequest(request);
    return m_d->processApiNetworkReplySingleObject<KimaiVersion>(ApiMethod::Version, reply);
}

QFuture<KimaiUser> KimaiClient::requestMeUserInfo()
{
    auto request = m_d->prepareRequest(ApiMethod::MeUsers);
    auto reply   = m_d->sendGetRequest(request);
    return m_d->processApiNetworkReplySingleObject<KimaiUser>(ApiMethod::MeUsers, reply);
}

QFuture<KimaiTimeSheetConfig> KimaiClient::requestTimeSheetConfig()
{
    auto request = m_d->prepareRequest(ApiMethod::TimeSheetConfig);
    auto reply   = m_d->sendGetRequest(request);
    return m_d->processApiNetworkReplySingleObject<KimaiTimeSheetConfig>(ApiMethod::TimeSheetConfig, reply);
}

QFuture<KimaiPlugins> KimaiClient::requestPlugins()
{
    auto request = m_d->prepareRequest(ApiMethod::Plugins);
    auto reply   = m_d->sendGetRequest(request);
    return m_d->processApiNetworkReplyArray<KimaiPlugin>(ApiMethod::Plugins, reply);
}

QFuture<KimaiCustomers> KimaiClient::requestCustomers()
{
    auto request = m_d->prepareRequest(ApiMethod::Customers);
    auto reply   = m_d->sendGetRequest(request);
    return m_d->processApiNetworkReplyArray<KimaiCustomer>(ApiMethod::Customers, reply);
}

QFuture<KimaiTimeSheets> KimaiClient::requestActiveTimeSheets()
{
    auto request = m_d->prepareRequest(ApiMethod::ActiveTimeSheets);
    auto reply   = m_d->sendGetRequest(request);
    return m_d->processApiNetworkReplyArray<KimaiTimeSheet>(ApiMethod::ActiveTimeSheets, reply);
}

QFuture<KimaiTimeSheets> KimaiClient::requestRecentTimeSheets()
{
    auto request = m_d->prepareRequest(ApiMethod::RecentTimeSheets, std::map<QString, QString>{
                                                                        {"size", "5"}
    });
    auto reply   = m_d->sendGetRequest(request);
    return m_d->processApiNetworkReplyArray<KimaiTimeSheet>(ApiMethod::RecentTimeSheets, reply);
}

QFuture<KimaiProjects> KimaiClient::requestProjects(std::optional<int> customerId)
{
    std::map<QString, QString> parameters;
    if (customerId.has_value())
    {
        parameters.emplace("customer", QString::number(customerId.value()));
    }

    auto request = m_d->prepareRequest(ApiMethod::Projects, parameters);
    auto reply   = m_d->sendGetRequest(request);
    return m_d->processApiNetworkReplyArray<KimaiProject>(ApiMethod::Projects, reply);
}

QFuture<KimaiActivities> KimaiClient::requestActivities(std::optional<int> projectId)
{
    std::map<QString, QString> parameters;
    if (projectId.has_value())
    {
        parameters.emplace("project", QString::number(projectId.value()));
    }

    auto request = m_d->prepareRequest(ApiMethod::Activities, parameters);
    auto reply   = m_d->sendGetRequest(request);
    return m_d->processApiNetworkReplyArray<KimaiActivity>(ApiMethod::Activities, reply);
}

QFuture<KimaiCustomer> KimaiClient::addCustomer(const KimaiCustomer &customer)
{
    auto data    = JsonHelpers::toByteArray(customer.toJson());
    auto request = m_d->prepareRequest(ApiMethod::CustomerAdd, {}, data);
    auto reply   = m_d->sendPostRequest(request, data);
    return m_d->processApiNetworkReplySingleObject<KimaiCustomer>(ApiMethod::CustomerAdd, reply);
}

QFuture<KimaiProject> KimaiClient::addProject(const KimaiProject &project)
{
    auto data    = JsonHelpers::toByteArray(project.toJson());
    auto request = m_d->prepareRequest(ApiMethod::ProjectAdd, {}, data);
    auto reply   = m_d->sendPostRequest(request, data);
    return m_d->processApiNetworkReplySingleObject<KimaiProject>(ApiMethod::ProjectAdd, reply);
}

QFuture<KimaiActivity> KimaiClient::addActivity(const KimaiActivity &activity)
{
    auto data    = JsonHelpers::toByteArray(activity.toJson());
    auto request = m_d->prepareRequest(ApiMethod::ActivityAdd, {}, data);
    auto reply   = m_d->sendPostRequest(request, data);
    return m_d->processApiNetworkReplySingleObject<KimaiActivity>(ApiMethod::ActivityAdd, reply);
}

QFuture<KimaiTimeSheet> KimaiClient::startTimeSheet(const KimaiTimeSheet &timeSheet, KimaiTimeSheetConfig::TrackingMode trackingMode)
{
    auto data    = JsonHelpers::toByteArray(timeSheet.toJson(trackingMode));
    auto request = m_d->prepareRequest(ApiMethod::TimeSheets, {}, data);
    auto reply   = m_d->sendPostRequest(request, data);
    return m_d->processApiNetworkReplySingleObject<KimaiTimeSheet>(ApiMethod::TimeSheets, reply);
}

QFuture<KimaiTimeSheet> KimaiClient::updateTimeSheet(const KimaiTimeSheet &timeSheet, KimaiTimeSheetConfig::TrackingMode trackingMode)
{
    auto data    = JsonHelpers::toByteArray(timeSheet.toJson(trackingMode));
    auto request = m_d->prepareRequest(ApiMethod::TimeSheets, {}, data, QString::number(timeSheet.id));
    auto reply   = m_d->sendPatchRequest(request, data);
    return m_d->processApiNetworkReplySingleObject<KimaiTimeSheet>(ApiMethod::TimeSheets, reply);
}

QFuture<KimaiTasks> KimaiClient::requestTasks()
{
    auto request = m_d->prepareRequest(ApiMethod::Tasks);
    auto reply   = m_d->sendGetRequest(request);
    return m_d->processApiNetworkReplyArray<KimaiTask>(ApiMethod::Tasks, reply);
}

QFuture<KimaiTask> KimaiClient::startTask(int taskId)
{
    auto request = m_d->prepareRequest(ApiMethod::TaskStart, {}, {}, QString("%1/start").arg(taskId));
    auto reply   = m_d->sendPatchRequest(request, {});
    return m_d->processApiNetworkReplySingleObject<KimaiTask>(ApiMethod::TaskStart, reply);
}

QFuture<KimaiTask> KimaiClient::closeTask(int taskId)
{
    auto request = m_d->prepareRequest(ApiMethod::TaskClose, {}, {}, QString("%1/close").arg(taskId));
    auto reply   = m_d->sendPatchRequest(request, {});
    return m_d->processApiNetworkReplySingleObject<KimaiTask>(ApiMethod::TaskClose, reply);
}

void KimaiClient::addTrustedCertificates(const QStringList &trustedCertificates)
{
    auto sslConfiguration = QSslConfiguration::defaultConfiguration();
    for (const auto &pemStr : trustedCertificates)
    {
        auto certificates = sslConfiguration.caCertificates();
        certificates << QSslCertificate::fromData(pemStr.toLocal8Bit());
        sslConfiguration.setCaCertificates(certificates);
    }
    QSslConfiguration::setDefaultConfiguration(sslConfiguration);
}
