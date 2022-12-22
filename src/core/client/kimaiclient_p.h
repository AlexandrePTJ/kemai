#pragma once

#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QVersionNumber>

#include "kimaiclient.h"

namespace kemai::client {

class KimaiClient::KimaiClientPrivate : public QObject
{
    Q_OBJECT

public:
    explicit KimaiClientPrivate(KimaiClient* c);

    QNetworkRequest prepareRequest(const KimaiRequest& cmd) const;
    QNetworkRequest prepareRequest(ApiMethod method, const std::map<QString, QString>& parameters = {}, const QByteArray& data = {},
                                   const QString& subPath = "") const;

    QNetworkReply* sendGetRequest(const QNetworkRequest& networkRequest);

public:
    QString username, host, token;
    QMap<QNetworkReply*, QSharedPointer<KimaiRequest>> runningRequests;

    QScopedPointer<QNetworkAccessManager> networkAccessManager;

private:
    void onNamFinished(QNetworkReply* reply);
    void onNamSslErrors(QNetworkReply* reply, const QList<QSslError>& errors);

    KimaiClient* const mQ;
};

} // namespace kemai::client
