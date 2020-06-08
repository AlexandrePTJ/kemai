#include "kemai/kimaiclient.h"
#include "kimaiclient_p.h"

#include <spdlog/spdlog.h>

using namespace kemai::client;

/*
 * Private impl
 */
KimaiClient::KimaiClientPrivate::KimaiClientPrivate(KimaiClient* c)
    : networkAccessManager(new QNetworkAccessManager), mQ(c)
{
    connect(networkAccessManager.data(), &QNetworkAccessManager::finished, this, &KimaiClientPrivate::onNamFinished);
}

QNetworkRequest KimaiClient::KimaiClientPrivate::prepareRequest(const KimaiRequest& cmd)
{
    QNetworkRequest r;
    r.setUrl(cmd.url(host));
    r.setRawHeader("X-AUTH-USER", username.toLatin1());
    r.setRawHeader("X-AUTH-TOKEN", token.toLatin1());
    if (cmd.httpVerb() == HttpVerb::Post)
        r.setRawHeader("Content-Type", "application/json");
    return r;
}

void KimaiClient::KimaiClientPrivate::onNamFinished(QNetworkReply* reply)
{
    auto kimaiRequest = runningRequests.take(reply);
    if (kimaiRequest)
    {
        if (reply->error() != QNetworkReply::NoError)
        {
            emit mQ->requestError(tr("Error on request [%1]: %2")
                                      .arg(apiMethodToString(kimaiRequest->method()))
                                      .arg(reply->errorString()));
        }
        else
        {
            auto replyData = reply->readAll();
            spdlog::debug("<=== {}", replyData.toStdString());
            emit mQ->replyReceived({kimaiRequest->method(), replyData});
        }
    }
    else
    {
        emit mQ->requestError(tr("Unknown request [%1]").arg(reply->errorString()));
    }
}

/*
 * Public impl
 */
KimaiClient::KimaiClient(QObject* parent) : QObject(parent), mD(new KimaiClientPrivate(this)) {}

KimaiClient::~KimaiClient() {}

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

    mD->runningRequests.insert(reply, QSharedPointer<KimaiRequest>::create(rq));
}
