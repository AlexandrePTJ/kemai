// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// 3rd party headers
#include <spdlog/spdlog.h>

// Qt headers
#include <QFuture>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QPromise>

// Project headers
#include <misc/customFmt.h>

// Local headers
#include "kimaiClient.h"

namespace kemai
{
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
        explicit KimaiClientPrivate(KimaiClient *c);

        QNetworkRequest prepareRequest(ApiMethod method, const std::map<QString, QString> &parameters = {}, const QByteArray &data = {}, const QString &subPath = "") const;

        QNetworkReply *sendGetRequest(const QNetworkRequest &networkRequest) const;
        QNetworkReply *sendPostRequest(const QNetworkRequest &networkRequest, const QByteArray &data) const;
        QNetworkReply *sendPatchRequest(const QNetworkRequest &networkRequest, const QByteArray &data) const;

        template<class ResultType>
        QFuture<ResultType> processApiNetworkReplySingleObject(ApiMethod method, QNetworkReply *networkReply)
        {
            QPromise<ResultType> promise;
            promise.start();
            auto future = promise.future();

            // clang-format off
            connect(
                networkReply,
                &QNetworkReply::finished,
                this,
                [networkReply, promise = std::move(promise), method]() mutable
                {
                    if (networkReply->error() == QNetworkReply::NoError)
                    {
                        try
                        {
                            spdlog::debug("[RECV] {}", apiMethodToString(method));

                            QJsonParseError parseError;
                            auto            doc = QJsonDocument::fromJson(networkReply->readAll(), &parseError);
                            if (parseError.error != QJsonParseError::NoError)
                            {
                                throw std::runtime_error(parseError.errorString().toStdString());
                            }

                            promise.addResult(ResultType::fromJson(doc.object()));
                        }
                        catch (...)
                        {
                            promise.setException(std::current_exception());
                        }
                    }
                    else
                    {
                        auto error = tr("Error on request [%1]: %2\n%3").arg(apiMethodToString(method), networkReply->errorString(), networkReply->readAll());
                        promise.setException(std::make_exception_ptr(std::runtime_error(error.toStdString())));
                    }
                    promise.finish();
                    networkReply->deleteLater();
                }
            );
            // clang-format on

            return future;
        }

        template<class ResultType>
        QFuture<QList<ResultType>> processApiNetworkReplyArray(ApiMethod method, QNetworkReply *networkReply)
        {
            QPromise<QList<ResultType>> promise;
            promise.start();
            auto future = promise.future();

            // clang-format off
            connect(
                networkReply,
                &QNetworkReply::finished,
                this,
                [networkReply, promise = std::move(promise), method]() mutable
                {
                    if (networkReply->error() == QNetworkReply::NoError)
                    {
                        try
                        {
                            spdlog::debug("[RECV] {}", apiMethodToString(method));

                            QJsonParseError parseError;
                            auto doc = QJsonDocument::fromJson(networkReply->readAll(), &parseError);
                            if (parseError.error != QJsonParseError::NoError)
                            {
                                throw std::runtime_error(parseError.errorString().toStdString());
                            }
                            if (!doc.isArray())
                            {
                                throw std::runtime_error("JSON response is not an array");
                            }

                            QList<ResultType> items;
                            for (const auto &val : doc.array())
                            {
                                items.push_back(ResultType::fromJson(val));
                            }
                            promise.addResult(std::move(items));
                        }
                        catch (...)
                        {
                            promise.setException(std::current_exception());
                        }
                    }
                    else
                    {
                        auto error = tr("Error on request [%1]: %2\n%3").arg(apiMethodToString(method), networkReply->errorString(), networkReply->readAll());
                        promise.setException(std::make_exception_ptr(std::runtime_error(error.toStdString())));
                    }
                    promise.finish();
                    networkReply->deleteLater();
                }
            );
            // clang-format on

            return future;
        }

        void onNamSslErrors(QNetworkReply *reply, const QList<QSslError> &errors);

        QString                                m_username, m_host, m_token, m_apiToken;
        std::unique_ptr<QNetworkAccessManager> m_networkAccessManager;

    private:
        KimaiClient *const m_q;
    };

} // namespace kemai
