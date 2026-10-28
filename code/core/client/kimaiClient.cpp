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

    case ApiMethod::TagsFind:
        return "tags/find";

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

    case ApiMethod::ColorsConfig:
        return "config/colors";

    case ApiMethod::MetaFields:
        return "metafields";

    default:
        return "";
    }
}

/*
 * Private impl
 */
KimaiClient::KimaiClientPrivate::KimaiClientPrivate(KimaiClient *c):
m_networkAccessManager(std::make_unique<QNetworkAccessManager>()),
m_q(c)
{
    m_networkAccessManager->setTransferTimeout(KimaiClient::DefaultTransferTimeout);
    connect(m_networkAccessManager.get(), &QNetworkAccessManager::sslErrors, this, &KimaiClientPrivate::onNamSslErrors);
}

QNetworkRequest KimaiClient::KimaiClientPrivate::prepareRequest(ApiMethod method, const std::map<QString, QString> &parameters, const QByteArray &data, const QString &subPath) const
{
    auto url  = QUrl::fromUserInput(m_host);
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
    if (m_apiToken.isEmpty())
    {
        networkRequest.setRawHeader("X-AUTH-USER", m_username.toUtf8());
        networkRequest.setRawHeader("X-AUTH-TOKEN", m_token.toUtf8());
    }
    else
    {
        networkRequest.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiToken).toLatin1());
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
    spdlog::debug("[GET] {}", networkRequest.url().toString(QUrl::RemoveUserInfo));
    return m_networkAccessManager->get(networkRequest);
}

QNetworkReply *KimaiClient::KimaiClientPrivate::sendPostRequest(const QNetworkRequest &networkRequest, const QByteArray &data) const
{
    spdlog::debug("[POST] {}", networkRequest.url().toString(QUrl::RemoveUserInfo));
    return m_networkAccessManager->post(networkRequest, data);
}

QNetworkReply *KimaiClient::KimaiClientPrivate::sendPatchRequest(const QNetworkRequest &networkRequest, const QByteArray &data) const
{
    spdlog::debug("[PATCH] {}", networkRequest.url().toString(QUrl::RemoveUserInfo));
    return m_networkAccessManager->sendCustomRequest(networkRequest, "PATCH", data);
}

QNetworkReply *KimaiClient::KimaiClientPrivate::sendDeleteRequest(const QNetworkRequest &networkRequest) const
{
    spdlog::debug("[DELETE] {}", networkRequest.url().toString(QUrl::RemoveUserInfo));
    return m_networkAccessManager->deleteResource(networkRequest);
}

namespace
{
    // Reply properties carrying the certificate problem to makeReplyError()
    constexpr auto PropertyCertificatePem = "kemaiCertificatePem";
    constexpr auto PropertyRejection      = "kemaiCertificateRejection";
    constexpr auto PropertyReplaces       = "kemaiCertificateReplacesTrusted";
} // namespace

KimaiApiError KimaiClient::KimaiClientPrivate::makeReplyError(ApiMethod method, QNetworkReply *networkReply)
{
    const auto httpStatus = networkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    auto kind = ApiErrorKind::Unknown;
    switch (httpStatus)
    {
    case 0: // no HTTP exchange happened: offline, timeout, DNS, connection refused...
        kind = ApiErrorKind::NoNetwork;
        break;
    case 401:
        kind = ApiErrorKind::AuthFailed;
        break;
    case 403:
        kind = ApiErrorKind::Forbidden;
        break;
    case 404:
        kind = ApiErrorKind::NotFound;
        break;
    case 400:
        kind = ApiErrorKind::InvalidValue;
        break;
    default:
        kind = httpStatus >= 500 ? ApiErrorKind::ServerError : ApiErrorKind::Unknown;
        break;
    }

    // Kimai returns errors as {"message": "..."}; validation errors (400) add the field messages
    auto       serverMessage = networkReply->errorString();
    QString    field;
    const auto doc = QJsonDocument::fromJson(networkReply->readAll());
    if (doc.isObject() && doc.object().contains("message"))
    {
        serverMessage = doc.object().value("message").toString();
    }
    if (kind == ApiErrorKind::InvalidValue)
    {
        if (const auto [errorField, message] = validationErrorFromJson(doc); !message.isEmpty())
        {
            field         = errorField;
            serverMessage = message;
        }
    }

    // TLS refused before any HTTP exchange: say which certificate and why
    if (const auto pem = networkReply->property(PropertyCertificatePem).toByteArray(); !pem.isEmpty())
    {
        auto info            = CertificateInfo::fromCertificate(QSslCertificate(pem), hostKey(networkReply->url()));
        info.rejection       = static_cast<CertificateInfo::Rejection>(networkReply->property(PropertyRejection).toInt());
        info.replacesTrusted = networkReply->property(PropertyReplaces).toBool();

        KimaiApiError error(info.rejection == CertificateInfo::Rejection::None ? ApiErrorKind::UntrustedCertificate : ApiErrorKind::CertificateRejected, 0, apiMethodToString(method), networkReply->errorString());
        error.setCertificate(info);
        return error;
    }

    return {kind, httpStatus, apiMethodToString(method), serverMessage, field};
}

QString KimaiClient::KimaiClientPrivate::httpsRedirectBase(QNetworkReply *networkReply)
{
    const auto requested = networkReply->request().url();
    const auto reached   = networkReply->url();
    if (requested.scheme() != "http" || reached.scheme() != "https")
    {
        return {};
    }

    // Keep what precedes /api/ (Kimai may live in a sub-path)
    auto       base = reached.adjusted(QUrl::RemoveQuery | QUrl::RemoveFragment | QUrl::RemoveUserInfo);
    const auto path = base.path();
    base.setPath(path.left(std::max<qsizetype>(0, path.indexOf("/api/"))));
    return base.toString();
}

KimaiApiError KimaiClient::KimaiClientPrivate::makeParseError(ApiMethod method, const QString &reason)
{
    return {ApiErrorKind::InvalidResponse, 0, apiMethodToString(method), reason};
}

namespace
{
    // Errors that only say the issuer is unknown: a certificate the user trusted for this host may override them
    bool isUnknownIssuer(QSslError::SslError error)
    {
        switch (error)
        {
        case QSslError::SelfSignedCertificate:
        case QSslError::SelfSignedCertificateInChain:
        case QSslError::UnableToGetLocalIssuerCertificate:
        case QSslError::UnableToGetIssuerCertificate:
        case QSslError::UnableToVerifyFirstCertificate:
        case QSslError::CertificateUntrusted:
            return true;
        default:
            return false;
        }
    }

    kemai::CertificateInfo::Rejection rejectionOf(QSslError::SslError error)
    {
        using Rejection = kemai::CertificateInfo::Rejection;
        switch (error)
        {
        case QSslError::CertificateExpired:
            return Rejection::Expired;
        case QSslError::CertificateNotYetValid:
            return Rejection::NotYetValid;
        case QSslError::HostNameMismatch:
            return Rejection::HostName;
        case QSslError::CertificateRevoked:
            return Rejection::Revoked;
        default:
            return Rejection::Other;
        }
    }

    // Expired or not yet valid by its own dates, whatever the TLS backend reported
    kemai::CertificateInfo::Rejection dateRejectionOf(const QSslCertificate &certificate)
    {
        using Rejection = kemai::CertificateInfo::Rejection;
        if (certificate.isNull())
        {
            return Rejection::None;
        }
        const auto now = QDateTime::currentDateTimeUtc();
        if (certificate.expiryDate() < now)
        {
            return Rejection::Expired;
        }
        if (certificate.effectiveDate() > now)
        {
            return Rejection::NotYetValid;
        }
        return Rejection::None;
    }

} // namespace

void KimaiClient::KimaiClientPrivate::onNamSslErrors(QNetworkReply *reply, const QList<QSslError> &errors)
{
    if (errors.isEmpty())
    {
        return;
    }

    auto leaf = reply->sslConfiguration().peerCertificate();
    if (leaf.isNull())
    {
        const auto withCertificate = std::ranges::find_if(errors, [](const QSslError &e)
                                                          { return !e.certificate().isNull(); });
        leaf                       = withCertificate != errors.cend() ? withCertificate->certificate() : QSslCertificate();
    }
    const auto host = hostKey(reply->url());
    const auto sha  = sha256Of(leaf);

    const auto rejection = certificateRejection(errors, leaf);
    const auto trusted   = std::ranges::find_if(m_trustedCertificates, [&host](const TrustedCertificate &t)
                                                { return t.host == host; });

    // Only an unknown issuer, and the user trusted exactly this certificate for this host
    if (rejection == CertificateInfo::Rejection::None && trusted != m_trustedCertificates.cend() && trusted->sha256 == sha)
    {
        reply->ignoreSslErrors(errors);
        return;
    }

    for (const auto &error : errors)
    {
        spdlog::error("SSL error on {}: {}", host, error.errorString());
    }

    reply->setProperty(PropertyCertificatePem, leaf.toPem());
    reply->setProperty(PropertyRejection, static_cast<int>(rejection));
    reply->setProperty(PropertyReplaces, trusted != m_trustedCertificates.cend());
}

/*
 * Public impl
 */
CertificateInfo::Rejection KimaiClient::certificateRejection(const QList<QSslError> &errors, const QSslCertificate &leaf)
{
    if (const auto byDate = dateRejectionOf(leaf); byDate != CertificateInfo::Rejection::None)
    {
        return byDate;
    }
    const auto blocking = std::ranges::find_if(errors, [](const QSslError &e)
                                               { return !isUnknownIssuer(e.error()); });
    return blocking != errors.cend() ? rejectionOf(blocking->error()) : CertificateInfo::Rejection::None;
}

KimaiClient::KimaiClient(QObject *parent):
QObject(parent),
m_d(std::make_unique<KimaiClientPrivate>(this))
{
}

KimaiClient::~KimaiClient() = default;

void KimaiClient::setHost(const QString &host)
{
    m_d->m_host = normalizedHost(host);
}

QString KimaiClient::normalizedHost(const QString &host)
{
    auto normalized = host.trimmed();
    if (!normalized.isEmpty() && !normalized.contains("://"))
    {
        normalized.prepend("https://");
    }
    while (normalized.endsWith('/'))
    {
        normalized.chop(1);
    }
    return normalized;
}

void KimaiClient::setTrustedCertificates(const TrustedCertificates &certificates)
{
    m_d->m_trustedCertificates = certificates;
}

void KimaiClient::setTransferTimeout(std::chrono::milliseconds timeout)
{
    m_d->m_networkAccessManager->setTransferTimeout(timeout);
}

void KimaiClient::setToken(const QString &token)
{
    m_d->m_apiToken = token;
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

QFuture<QStringList> KimaiClient::requestTags(const QString &name)
{
    auto request = m_d->prepareRequest(ApiMethod::TagsFind, {
                                                                {"name", name}
    });
    auto reply   = m_d->sendGetRequest(request);
    return m_d->processApiNetworkReply<QStringList>(ApiMethod::TagsFind, reply, &KimaiClient::tagNamesFromJson);
}

std::pair<QString, QString> KimaiClient::validationErrorFromJson(const QJsonDocument &doc)
{
    const auto errors = doc.object().value("errors").toObject();

    const auto children = errors.value("children").toObject();
    for (auto it = children.begin(); it != children.end(); ++it)
    {
        const auto messages = it.value().toObject().value("errors").toArray();
        if (!messages.isEmpty())
        {
            return {it.key(), messages.first().toString()};
        }
    }

    // Form-level error, e.g. "This form should not contain extra fields."
    if (const auto messages = errors.value("errors").toArray(); !messages.isEmpty())
    {
        return {QString(), messages.first().toString()};
    }
    return {};
}

QStringList KimaiClient::tagNamesFromJson(const QJsonDocument &doc)
{
    if (!doc.isArray())
    {
        throw std::runtime_error("JSON response is not an array");
    }

    // tags/find returns tag objects; older endpoints return plain names
    QStringList names;
    for (const auto &value : doc.array())
    {
        const auto name = value.isObject() ? value.toObject().value("name").toString() : value.toString();
        if (!name.isEmpty())
        {
            names.append(name);
        }
    }
    return names;
}

QFuture<KimaiPlugins> KimaiClient::requestPlugins()
{
    auto request = m_d->prepareRequest(ApiMethod::Plugins);
    auto reply   = m_d->sendGetRequest(request);
    return m_d->processApiNetworkReplyArray<KimaiPlugin>(ApiMethod::Plugins, reply);
}

QFuture<KimaiMetaFieldRules> KimaiClient::requestTimeSheetMetaFields()
{
    auto request = m_d->prepareRequest(ApiMethod::MetaFields, {
                                                                  {"entity", "timesheet"}
    });
    auto reply   = m_d->sendGetRequest(request);
    return m_d->processApiNetworkReplyArray<KimaiMetaFieldRule>(ApiMethod::MetaFields, reply);
}

QFuture<KimaiColorChoices> KimaiClient::requestColorChoices()
{
    auto request = m_d->prepareRequest(ApiMethod::ColorsConfig);
    auto reply   = m_d->sendGetRequest(request);
    return m_d->processApiNetworkReply<KimaiColorChoices>(ApiMethod::ColorsConfig, reply, &KimaiClient::colorChoicesFromJson);
}

KimaiColorChoices KimaiClient::colorChoicesFromJson(const QJsonDocument &doc)
{
    if (!doc.isObject())
    {
        throw std::runtime_error("JSON response is not an object");
    }

    // A JSON object has no order once parsed: the palette comes sorted by name, not in Kimai's order
    KimaiColorChoices colors;
    const auto        object = doc.object();
    for (auto it = object.begin(); it != object.end(); ++it)
    {
        if (const auto color = it.value().toString(); !color.isEmpty())
        {
            colors.append({.name = it.key(), .color = color});
        }
    }
    return colors;
}

QFuture<KimaiCustomers> KimaiClient::requestCustomers()
{
    auto request = m_d->prepareRequest(ApiMethod::Customers);
    auto reply   = m_d->sendGetRequest(request);
    return m_d->processApiNetworkReplyArray<KimaiCustomer>(ApiMethod::Customers, reply);
}

QFuture<KimaiTimeSheets> KimaiClient::requestTimeSheets()
{
    std::map<QString, QString> parameters = {
        {"full",    "true" },
        {"order",   "DESC" },
        {"orderBy", "begin"}
    };

    auto request = m_d->prepareRequest(ApiMethod::TimeSheets, parameters);
    auto reply   = m_d->sendGetRequest(request);
    return m_d->processApiNetworkReplyArray<KimaiTimeSheet>(ApiMethod::TimeSheets, reply);
}

QFuture<KimaiTimeSheets> KimaiClient::requestActiveTimeSheets()
{
    auto request = m_d->prepareRequest(ApiMethod::ActiveTimeSheets);
    auto reply   = m_d->sendGetRequest(request);
    return m_d->processApiNetworkReplyArray<KimaiTimeSheet>(ApiMethod::ActiveTimeSheets, reply);
}

QFuture<KimaiTimeSheets> KimaiClient::requestRecentTimeSheets()
{
    auto request = m_d->prepareRequest(ApiMethod::RecentTimeSheets);
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

QFuture<void> KimaiClient::createTag(const QString &name)
{
    // Visible, or Kimai hides it from its own tag lists
    auto data    = JsonHelpers::toByteArray(QJsonObject{
        {"name",    name},
        {"visible", true}
    });
    auto request = m_d->prepareRequest(ApiMethod::Tags, {}, data);
    auto reply   = m_d->sendPostRequest(request, data);
    return m_d->processApiNetworkReplyNoContent(ApiMethod::Tags, reply);
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

QFuture<KimaiTimeSheet> KimaiClient::setTimeSheetMeta(int timeSheetId, const QString &name, const QString &value)
{
    auto data    = JsonHelpers::toByteArray(QJsonObject{
        {"name",  name },
        {"value", value}
    });
    auto request = m_d->prepareRequest(ApiMethod::TimeSheets, {}, data, QString("%1/meta").arg(timeSheetId));
    auto reply   = m_d->sendPatchRequest(request, data);
    return m_d->processApiNetworkReplySingleObject<KimaiTimeSheet>(ApiMethod::TimeSheets, reply);
}

QFuture<void> KimaiClient::deleteTimeSheet(int timeSheetId)
{
    auto request = m_d->prepareRequest(ApiMethod::TimeSheets, {}, {}, QString::number(timeSheetId));
    auto reply   = m_d->sendDeleteRequest(request);
    return m_d->processApiNetworkReplyNoContent(ApiMethod::TimeSheets, reply);
}

QFuture<KimaiTimeSheet> KimaiClient::stopTimeSheet(int timeSheetId)
{
    auto request = m_d->prepareRequest(ApiMethod::TimeSheets, {}, {}, QString("%1/stop").arg(timeSheetId));
    auto reply   = m_d->sendPatchRequest(request, {});
    return m_d->processApiNetworkReplySingleObject<KimaiTimeSheet>(ApiMethod::TimeSheets, reply);
}

QFuture<KimaiTasks> KimaiClient::requestTasks(int userId)
{
    // Default statuses: pending and in progress. One page holds them all in practice (default size is 50).
    auto request = m_d->prepareRequest(ApiMethod::Tasks, {
                                                             {"users[]", QString::number(userId)},
                                                             {"size",    "500"                  }
    });
    auto reply   = m_d->sendGetRequest(request);
    return m_d->processApiNetworkReplyArray<KimaiTask>(ApiMethod::Tasks, reply);
}

QFuture<KimaiTimeSheet> KimaiClient::startTask(int taskId)
{
    auto request = m_d->prepareRequest(ApiMethod::TaskStart, {}, {}, QString("%1/start").arg(taskId));
    auto reply   = m_d->sendPatchRequest(request, {});
    // The API documentation says Task; Kimai 2.70 answers with the started timesheet
    return m_d->processApiNetworkReplySingleObject<KimaiTimeSheet>(ApiMethod::TaskStart, reply);
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
