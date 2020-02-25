#include "kimaiclient.h"
#include "kimaiclient_p.h"

#include <QNetworkRequest>

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
    r.setRawHeader("accept", "application/json");
    if (cmd.httpVerb() == HttpVerb::Post)
        r.setRawHeader("content-type", "application/json");
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
            emit mQ->replyReceived({kimaiRequest->method(), reply->readAll()});
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
        reply = mD->networkAccessManager->get(qreq);
        break;

    case HttpVerb::Post:
        reply = mD->networkAccessManager->post(qreq, rq.data());
        break;

    case HttpVerb ::Patch:
        reply = mD->networkAccessManager->sendCustomRequest(qreq, "PATCH", rq.data());
        break;
    }

    mD->runningRequests.insert(reply, QSharedPointer<KimaiRequest>::create(rq));
}
