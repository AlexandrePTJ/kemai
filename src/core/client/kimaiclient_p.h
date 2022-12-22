#pragma once

#include <spdlog/spdlog.h>

#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QVersionNumber>

#include "kimaiclient.h"
#include "parser.h"

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

    template<class R, class T> std::shared_ptr<R> processApiNetworkReplySingleObject(ApiMethod method, QNetworkReply* networkReply)
    {
        auto result = std::make_shared<R>();

        QObject::connect(networkReply, &QNetworkReply::finished, this, [networkReply, result, method]() {
            if (networkReply->error() == QNetworkReply::NoError)
            {
                try
                {
                    auto replyData = networkReply->readAll();
                    spdlog::debug("<=== {}", replyData.toStdString());

                    KimaiApiTypesParser parser(replyData);
                    // TODO: find a way to deduct type from R
                    auto resultValue = parser.getValueOf<T>();
                    result->setResult(resultValue);
                }
                catch (std::runtime_error& ex)
                {
                    result->setError(ex.what());
                }
            }
            else
            {
                auto error = tr("Error on request [%1]: %2\n%3").arg(apiMethodToString(method), networkReply->errorString(), networkReply->readAll());
                result->setError(error);
            }
            networkReply->deleteLater();
        });

        return result;
    }

    template<class R, class T> std::shared_ptr<R> processApiNetworkReplyArray(ApiMethod method, QNetworkReply* networkReply)
    {
        auto result = std::make_shared<R>();

        QObject::connect(networkReply, &QNetworkReply::finished, this, [networkReply, result, method]() {
            if (networkReply->error() == QNetworkReply::NoError)
            {
                try
                {
                    auto replyData = networkReply->readAll();
                    spdlog::debug("<=== {}", replyData.toStdString());

                    KimaiApiTypesParser parser(replyData);
                    // TODO: find a way to deduct type from R
                    auto resultValue = parser.getArrayOf<T>();
                    result->setResult(resultValue);
                }
                catch (std::runtime_error& ex)
                {
                    result->setError(ex.what());
                }
            }
            else
            {
                auto error = tr("Error on request [%1]: %2\n%3").arg(apiMethodToString(method), networkReply->errorString(), networkReply->readAll());
                result->setError(error);
            }
            networkReply->deleteLater();
        });

        return result;
    }

    QString username, host, token;
    QMap<QNetworkReply*, QSharedPointer<KimaiRequest>> runningRequests;

    QScopedPointer<QNetworkAccessManager> networkAccessManager;

private:
    void onNamSslErrors(QNetworkReply* reply, const QList<QSslError>& errors);

    KimaiClient* const mQ;
};

} // namespace kemai::client
