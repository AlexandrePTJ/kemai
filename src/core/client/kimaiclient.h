#pragma once

#include <QObject>

#include "kimaireply.h"
#include "kimairequest.h"

namespace kemai::client {

class KimaiClient : public QObject
{
    Q_OBJECT

public:
    explicit KimaiClient(QObject* parent = nullptr);
    ~KimaiClient() override;

    void setHost(const QString& host);
    void setUsername(const QString& username);
    void setToken(const QString& token);

    void sendRequest(const KimaiRequest& rq);

    static void addTrustedCertificates(const QStringList& trustedCertificates);

signals:
    void requestError(const QString& errorMsg);
    void sslError(const QString& msg, const QByteArray& certSN, const QByteArray& certPem);
    void replyReceived(const kemai::client::KimaiReply& reply);

private:
    class KimaiClientPrivate;
    QScopedPointer<KimaiClientPrivate> mD;
};

} // namespace kemai::client
