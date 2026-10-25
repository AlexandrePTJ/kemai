// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// 3rd party headers
#include <spdlog/spdlog.h>

// STL headers
#include <type_traits>

// Qt headers
#include <QFuture>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QPromise>

// Project headers
#include <misc/customFmt.h>

// Local headers
#include <client/kimaiApiError.h>
#include <client/kimaiClient.h>

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
        TagsFind,
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
        QNetworkReply *sendDeleteRequest(const QNetworkRequest &networkRequest) const;

        static KimaiApiError makeReplyError(ApiMethod method, QNetworkReply *networkReply);
        static KimaiApiError makeParseError(ApiMethod method, const QString &reason);
        // Base URL (scheme, host, port, Kimai path) of the https:// address an http:// request was redirected to,
        // empty when not redirected that way
        static QString httpsRedirectBase(QNetworkReply *networkReply);

        template<class ResultType>
        QFuture<ResultType> processApiNetworkReplySingleObject(ApiMethod method, QNetworkReply *networkReply)
        {
            return processApiNetworkReply<ResultType>(method, networkReply, [](const QJsonDocument &doc)
                                                      { return ResultType::fromJson(doc.object()); });
        }

        template<class ResultType>
        QFuture<QList<ResultType>> processApiNetworkReplyArray(ApiMethod method, QNetworkReply *networkReply)
        {
            return processApiNetworkReply<QList<ResultType>>(method,
                                                             networkReply,
                                                             [](const QJsonDocument &doc)
                                                             {
                                                                 if (!doc.isArray())
                                                                 {
                                                                     throw std::runtime_error("JSON response is not an array");
                                                                 }

                                                                 QList<ResultType> items;
                                                                 for (const auto &val : doc.array())
                                                                 {
                                                                     items.push_back(ResultType::fromJson(val));
                                                                 }
                                                                 return items;
                                                             });
        }

        // For replies without body (e.g. DELETE answering 204 No Content)
        QFuture<void> processApiNetworkReplyNoContent(ApiMethod method, QNetworkReply *networkReply)
        {
            return processApiNetworkReply<void>(method, networkReply, [](const QJsonDocument &) {});
        }

        /*!
         * Resolves a future from a network reply: `parse` turns the JSON body into the result
         * (unused when ResultType is void). Transport/HTTP errors and parse exceptions are set
         * on the future as a KimaiApiError.
         */
        template<class ResultType, class Parser>
        QFuture<ResultType> processApiNetworkReply(ApiMethod method, QNetworkReply *networkReply, Parser parse)
        {
            QPromise<ResultType> promise;
            promise.start();
            auto future = promise.future();

            connect(
                networkReply,
                &QNetworkReply::finished,
                this,
                [networkReply, promise = std::move(promise), method, parse = std::move(parse)]() mutable
                {
                    // finish() may run continuations that release the last KimaiClient reference, destroying the
                    // network manager, this reply and this lambda: everything is done before it, nothing after.
                    auto result = std::move(promise);
                    networkReply->deleteLater();

                    if (const auto httpsBase = httpsRedirectBase(networkReply); !httpsBase.isEmpty())
                    {
                        result.setException(std::make_exception_ptr(
                            KimaiApiError{ApiErrorKind::HttpsRedirect, 0, apiMethodToString(method), httpsBase}));
                    }
                    else if (networkReply->error() == QNetworkReply::NoError)
                    {
                        try
                        {
                            spdlog::debug("[RECV] {}", apiMethodToString(method));

                            if constexpr (!std::is_void_v<ResultType>)
                            {
                                QJsonParseError parseError;
                                auto            doc = QJsonDocument::fromJson(networkReply->readAll(), &parseError);
                                if (parseError.error != QJsonParseError::NoError)
                                {
                                    throw std::runtime_error(parseError.errorString().toStdString());
                                }

                                result.addResult(parse(doc));
                            }
                        }
                        catch (const std::exception &e)
                        {
                            result.setException(std::make_exception_ptr(makeParseError(method, QString::fromUtf8(e.what()))));
                        }
                    }
                    else
                    {
                        result.setException(std::make_exception_ptr(makeReplyError(method, networkReply)));
                    }
                    result.finish();
                });

            return future;
        }

        void onNamSslErrors(QNetworkReply *reply, const QList<QSslError> &errors);

        QString                                m_username, m_host, m_token, m_apiToken;
        TrustedCertificates                    m_trustedCertificates;
        std::unique_ptr<QNetworkAccessManager> m_networkAccessManager;

    private:
        KimaiClient *const m_q;
    };

} // namespace kemai
