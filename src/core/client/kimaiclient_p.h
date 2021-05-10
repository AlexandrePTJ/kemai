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
    bool isRequestAvailableForCurrentInstance(ApiMethod apiMethod) const;
    void processReply(const KimaiReply& kimaiReply);

public:
    QString username, host, token;
    QMap<QNetworkReply*, QSharedPointer<KimaiRequest>> runningRequests;

    QScopedPointer<QNetworkAccessManager> networkAccessManager;

    // Fetched from version and plugins request.
    // Kept here to filter requests.
    QVersionNumber kimaiVersion;
    Plugins kimaiPlugins;

private slots:
    void onNamFinished(QNetworkReply* reply);
    void onNamSslErrors(QNetworkReply* reply, const QList<QSslError>& errors);

private:
    KimaiClient* const mQ;
};

} // namespace kemai::client
