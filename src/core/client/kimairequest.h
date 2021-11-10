#pragma once

#include <QMap>
#include <QScopedPointer>
#include <QString>
#include <QUrl>

#include "kimaiapi.h"

namespace kemai::client {

class KimaiRequest
{
public:
    KimaiRequest() = delete;
    KimaiRequest(const KimaiRequest& rhs);
    KimaiRequest(KimaiRequest&& rhs) noexcept;

    KimaiRequest& operator=(const KimaiRequest& rhs);
    KimaiRequest& operator=(KimaiRequest&& rhs) noexcept;

    virtual ~KimaiRequest();

    ApiMethod method() const;

    HttpVerb httpVerb() const;

    QByteArray data() const;

    QUrl url(const QString& host) const;

private:
    class KimaiRequestPrivate;
    QScopedPointer<KimaiRequestPrivate> mD;

    friend class KimaiRequestFactory;
    explicit KimaiRequest(const ApiMethod& method, HttpVerb verb = HttpVerb::Get);

    void setParameters(const QMap<QString, QString>& parameters);
    void setData(const QByteArray& data);
    void setPatchVerb(const QString& verb);
};

} // namespace kemai::client
