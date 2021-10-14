#include "kimairequest.h"

#include <QUrlQuery>

using namespace kemai::client;

class KimaiRequest::KimaiRequestPrivate
{
public:
    ApiMethod method = ApiMethod::Undefined;
    HttpVerb verb    = HttpVerb::Get;
    QMap<QString, QString> parameters;
    QByteArray data;
    QString patchVerb;
};

KimaiRequest::KimaiRequest(const ApiMethod& method, HttpVerb verb) : mD(new KimaiRequestPrivate)
{
    mD->method = method;
    mD->verb   = verb;
}

KimaiRequest::KimaiRequest(const KimaiRequest& rhs) : KimaiRequest(rhs.mD->method, rhs.mD->verb) {}

KimaiRequest::KimaiRequest(KimaiRequest&& rhs) noexcept : mD(new KimaiRequestPrivate)
{
    mD->method     = rhs.mD->method;
    mD->verb       = rhs.mD->verb;
    mD->parameters = std::move(rhs.mD->parameters);
    mD->data       = std::move(rhs.mD->data);
    mD->patchVerb  = rhs.mD->patchVerb;
}

KimaiRequest& KimaiRequest::operator=(const KimaiRequest& rhs)
{
    mD->method     = rhs.mD->method;
    mD->verb       = rhs.mD->verb;
    mD->parameters = rhs.mD->parameters;
    mD->data       = rhs.mD->data;
    mD->patchVerb  = rhs.mD->patchVerb;
    return *this;
}

KimaiRequest& KimaiRequest::operator=(KimaiRequest&& rhs) noexcept
{
    mD->method     = rhs.mD->method;
    mD->verb       = rhs.mD->verb;
    mD->parameters = std::move(rhs.mD->parameters);
    mD->data       = std::move(rhs.mD->data);
    mD->patchVerb  = rhs.mD->patchVerb;
    return *this;
}

KimaiRequest::~KimaiRequest() = default;

ApiMethod KimaiRequest::method() const
{
    return mD->method;
}

HttpVerb KimaiRequest::httpVerb() const
{
    return mD->verb;
}

QByteArray KimaiRequest::data() const
{
    return mD->data;
}

QUrl KimaiRequest::url(const QString& host) const
{
    auto url = QUrl::fromUserInput(host);

    auto path = url.path();
    if (!path.endsWith('/'))
    {
        path += '/';
    }

    path += "api/" + apiMethodToString(mD->method);
    if (!mD->patchVerb.isEmpty())
    {
        path += "/" + mD->patchVerb;
    }
    url.setPath(path);

    if (!mD->parameters.isEmpty())
    {
        QUrlQuery query;
        for (auto it = mD->parameters.begin(); it != mD->parameters.end(); ++it)
        {
            query.addQueryItem(it.key(), it.value());
        }
        url.setQuery(query);
    }

    return url;
}

void KimaiRequest::setParameters(const QMap<QString, QString>& parameters)
{
    mD->parameters = parameters;
}

void KimaiRequest::setData(const QByteArray& data)
{
    mD->data = data;
}

void KimaiRequest::setPatchVerb(const QString& verb)
{
    mD->patchVerb = verb;
}
