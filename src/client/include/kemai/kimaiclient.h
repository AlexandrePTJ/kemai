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
    virtual ~KimaiClient();

    void setHost(const QString& host);
    void setUsername(const QString& username);
    void setToken(const QString& token);

    void sendRequest(const KimaiRequest& rq);

signals:
    void requestError(const QString& errorMsg);
    void replyReceived(const KimaiReply& reply);

private:
    class KimaiClientPrivate;
    QScopedPointer<KimaiClientPrivate> mD;
};

} // namespace kemai::client
