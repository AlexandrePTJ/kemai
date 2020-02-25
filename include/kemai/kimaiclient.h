#pragma once

#include <QObject>

#include <kemai/kemai_export.h>
#include <kemai/kimaireply.h>
#include <kemai/kimairequest.h>

namespace kemai::client {

class KEMAI_EXPORT KimaiClient : public QObject
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
