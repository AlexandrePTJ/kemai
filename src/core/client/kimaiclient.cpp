#include "kimaiclient.h"
#include "kimaiclient_p.h"

#include <spdlog/spdlog.h>

using namespace kemai::client;

/*
 * Static infos
 */
static const auto PluginsApiMethodKimaiMinimalVersion = QVersionNumber(1, 14); // TODO: set to 1.15 once released

static const auto TaskPluginKimaiMinimalVersion  = QVersionNumber(1, 14); // TODO: set to 1.15 once released
static const auto TaskPluginBundleMinimalVersion = QVersionNumber(1, 10);
static const auto TaskPluginBundleName           = QString("TaskManagementBundle");
static const auto TaskPluginApiMethods           = QVector{ApiMethod::Tasks};

/*
 * Private impl
 */
KimaiClient::KimaiClientPrivate::KimaiClientPrivate(KimaiClient* c) : networkAccessManager(new QNetworkAccessManager), mQ(c)
{
    connect(networkAccessManager.data(), &QNetworkAccessManager::finished, this, &KimaiClientPrivate::onNamFinished);
    connect(networkAccessManager.data(), &QNetworkAccessManager::sslErrors, this, &KimaiClientPrivate::onNamSslErrors);
}

QNetworkRequest KimaiClient::KimaiClientPrivate::prepareRequest(const KimaiRequest& cmd) const
{
    QNetworkRequest r;
    r.setUrl(cmd.url(host));
    r.setRawHeader("X-AUTH-USER", username.toLatin1());
    r.setRawHeader("X-AUTH-TOKEN", token.toLatin1());
    if (cmd.httpVerb() == HttpVerb::Post)
        r.setRawHeader("Content-Type", "application/json");
    return r;
}

bool KimaiClient::KimaiClientPrivate::isRequestAvailableForCurrentInstance(ApiMethod apiMethod) const
{
    // Requires Kimai >= 1.15
    if (apiMethod == ApiMethod::Plugins)
    {
        return !kimaiVersion.isNull() && kimaiVersion >= PluginsApiMethodKimaiMinimalVersion;
    }

    // Requires Kimai >= 1.15 and TaskManagementBundle plugin >= 1.10
    if (TaskPluginApiMethods.contains(apiMethod))
    {
        if (!kimaiVersion.isNull() && kimaiVersion >= TaskPluginKimaiMinimalVersion)
        {
            for (const auto& plugin : kimaiPlugins)
            {
                if (plugin.name == TaskPluginBundleName)
                {
                    return QVersionNumber::fromString(plugin.version) >= TaskPluginBundleMinimalVersion;
                }
            }
        }
    }

    return true;
}

void KimaiClient::KimaiClientPrivate::processReply(const KimaiReply& kimaiReply)
{
    // Keep some infos.
    switch (kimaiReply.method())
    {
    case ApiMethod::Version: {
        auto kVersion = kimaiReply.get<KimaiVersion>();
        kimaiVersion  = QVersionNumber::fromString(kVersion.kimai);
    }
    break;

    case ApiMethod::Plugins: {
        kimaiPlugins = kimaiReply.get<Plugins>();
    }
    break;

    default:
        break;
    }

    // Send to GUI
    emit mQ->replyReceived(kimaiReply);
}

void KimaiClient::KimaiClientPrivate::onNamFinished(QNetworkReply* reply)
{
    auto kimaiRequest = runningRequests.take(reply);
    if (kimaiRequest)
    {
        if (reply->error() != QNetworkReply::NoError)
        {
            emit mQ->requestError(tr("Error on request [%1]: %2").arg(apiMethodToString(kimaiRequest->method()), reply->errorString()));
        }
        else
        {
            auto replyData = reply->readAll();
            spdlog::debug("<=== {}", replyData.toStdString());
            processReply({kimaiRequest->method(), replyData});
        }
    }
    else
    {
        emit mQ->requestError(tr("Unknown request [%1]").arg(reply->errorString()));
    }
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

void KimaiClient::sendRequest(const KimaiRequest& rq)
{
    if (!mD->isRequestAvailableForCurrentInstance(rq.method()))
    {
        spdlog::error("Method {} is not available for your Kimai instance.", apiMethodToString(rq.method()).toStdString());
        return;
    }

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

    if (reply)
        mD->runningRequests.insert(reply, QSharedPointer<KimaiRequest>::create(rq));
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
