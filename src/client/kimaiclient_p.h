#pragma once

#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>

#include "kemai/kimaiclient.h"

namespace kemai::client {

class KimaiClient::KimaiClientPrivate : public QObject
{
    Q_OBJECT

public:
    explicit KimaiClientPrivate(KimaiClient* c);

    QNetworkRequest prepareRequest(const KimaiRequest& cmd);

public:
    QString username, host, token;
    QMap<QNetworkReply*, QSharedPointer<KimaiRequest>> runningRequests;

    QScopedPointer<QNetworkAccessManager> networkAccessManager;

private slots:
    void onNamFinished(QNetworkReply* reply);

private:
    KimaiClient* const mQ;
};

} // namespace kemai::client
