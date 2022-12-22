#include "kimaiclient.h"
#include "kimaiclient_p.h"

#include "kemai_version.h"

#include <QCoreApplication>
#include <QUrlQuery>

#include <spdlog/spdlog.h>

using namespace kemai::client;

/*
 * Private impl
 */
KimaiClient::KimaiClientPrivate::KimaiClientPrivate(KimaiClient* c) : networkAccessManager(new QNetworkAccessManager), mQ(c)
{
    connect(networkAccessManager.data(), &QNetworkAccessManager::sslErrors, this, &KimaiClientPrivate::onNamSslErrors);
}

QNetworkRequest KimaiClient::KimaiClientPrivate::prepareRequest(const KimaiRequest& cmd) const
{
    QNetworkRequest r;
    r.setUrl(cmd.url(host));
    r.setRawHeader("X-AUTH-USER", username.toLatin1());
    r.setRawHeader("X-AUTH-TOKEN", token.toLatin1());
    r.setHeader(QNetworkRequest::UserAgentHeader, QString("%1/%2").arg(qApp->applicationName(), KEMAI_VERSION));
    r.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    if (cmd.httpVerb() == HttpVerb::Post)
    {
        r.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        r.setHeader(QNetworkRequest::ContentLengthHeader, cmd.data().size());
    }

    return r;
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

QNetworkReply* KimaiClient::KimaiClientPrivate::sendGetRequest(const QNetworkRequest& networkRequest)
{
    spdlog::debug("===> [GET] {}", networkRequest.url().toString().toStdString());
    return networkAccessManager->get(networkRequest);
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

std::shared_ptr<VersionRequestResult> KimaiClient::requestKimaiVersion()
{
    auto request = mD->prepareRequest(ApiMethod::Version);
    auto reply   = mD->sendGetRequest(request);
    return mD->processApiNetworkReplySingleObject<VersionRequestResult, KimaiVersion>(ApiMethod::Version, reply);
}

std::shared_ptr<MeRequestResult> KimaiClient::requestMeUserInfo()
{
    auto request = mD->prepareRequest(ApiMethod::MeUsers);
    auto reply   = mD->sendGetRequest(request);
    return mD->processApiNetworkReplySingleObject<MeRequestResult, User>(ApiMethod::MeUsers, reply);
}

std::shared_ptr<TimeSheetConfigResult> KimaiClient::requestTimeSheetConfig()
{
    auto request = mD->prepareRequest(ApiMethod::TimeSheetConfig);
    auto reply   = mD->sendGetRequest(request);
    return mD->processApiNetworkReplySingleObject<TimeSheetConfigResult, TimeSheetConfig>(ApiMethod::TimeSheetConfig, reply);
}

std::shared_ptr<PluginsResult> KimaiClient::requestPlugins()
{
    auto request = mD->prepareRequest(ApiMethod::Plugins);
    auto reply   = mD->sendGetRequest(request);
    return mD->processApiNetworkReplyArray<PluginsResult, Plugin>(ApiMethod::Plugins, reply);
}

void KimaiClient::sendRequest(const KimaiRequest& rq)
{
    auto qreq = mD->prepareRequest(rq);

    QNetworkReply* reply = nullptr;
    switch (rq.httpVerb())
    {
    case HttpVerb::Get:
        spdlog::debug("===> [GET] {}", qreq.url().toString().toStdString());
        reply = mD->networkAccessManager->get(qreq);
        break;

    case HttpVerb::Post:
        spdlog::debug("===> [POST] {} {}", qreq.url().toString().toStdString(), rq.data().toStdString());
        reply = mD->networkAccessManager->post(qreq, rq.data());
        break;

    case HttpVerb ::Patch:
        spdlog::debug("===> [PATCH] {}", qreq.url().toString().toStdString());
        reply = mD->networkAccessManager->sendCustomRequest(qreq, "PATCH", rq.data());
        break;
    }

    if (reply != nullptr)
    {
        mD->runningRequests.insert(reply, QSharedPointer<KimaiRequest>::create(rq));
    }
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
