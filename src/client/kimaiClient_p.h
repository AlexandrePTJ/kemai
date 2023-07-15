#pragma once

#include <spdlog/spdlog.h>

#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QVersionNumber>

#include "kimaiClient.h"
#include "parser.h"

namespace kemai {

// available requests
enum class ApiMethod
{
    Version,
    Customers,
    CustomerAdd,
    Projects,
    ProjectAdd,
    Activities,
    ActivityAdd,
    ActiveTimeSheets,
    RecentTimeSheets,
    TimeSheets,
    Users,
    MeUsers,
    Tags,
    Plugins,
    Tasks,
    TaskLog,
    TaskStart,
    TaskStop,
    TaskClose,
    TimeSheetConfig
};
QString apiMethodToString(ApiMethod method);

class KimaiClient::KimaiClientPrivate : public QObject
{
    Q_OBJECT

public:
    explicit KimaiClientPrivate(KimaiClient* c);

    QNetworkRequest prepareRequest(ApiMethod method, const std::map<QString, QString>& parameters = {}, const QByteArray& data = {},
                                   const QString& subPath = "") const;

    QNetworkReply* sendGetRequest(const QNetworkRequest& networkRequest) const;
    QNetworkReply* sendPostRequest(const QNetworkRequest& networkRequest, const QByteArray& data) const;
    QNetworkReply* sendPatchRequest(const QNetworkRequest& networkRequest, const QByteArray& data) const;

    template<class ResultType> KimaiApiResult<ResultType>* processApiNetworkReplySingleObject(ApiMethod method, QNetworkReply* networkReply)
    {
        auto result = new KimaiApiResult<ResultType>;

        QObject::connect(networkReply, &QNetworkReply::finished, this, [networkReply, result, method]() {
            if (networkReply->error() == QNetworkReply::NoError)
            {
                try
                {
                    spdlog::debug("[RECV] {}", apiMethodToString(method).toStdString());

                    KimaiApiTypesParser parser(networkReply->readAll());
                    result->setResult(parser.getValueOf<ResultType>());
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

    template<class ResultType> KimaiApiResult<std::vector<ResultType>>* processApiNetworkReplyArray(ApiMethod method, QNetworkReply* networkReply)
    {
        auto result = new KimaiApiResult<std::vector<ResultType>>;

        QObject::connect(networkReply, &QNetworkReply::finished, this, [networkReply, result, method]() {
            if (networkReply->error() == QNetworkReply::NoError)
            {
                try
                {
                    spdlog::debug("[RECV] {}", apiMethodToString(method).toStdString());

                    KimaiApiTypesParser parser(networkReply->readAll());
                    result->setResult(parser.getArrayOf<ResultType>());
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

    void onNamSslErrors(QNetworkReply* reply, const QList<QSslError>& errors);

    QString username, host, token;
    QScopedPointer<QNetworkAccessManager> networkAccessManager;

private:
    KimaiClient* const mQ;
};

} // namespace kemai
