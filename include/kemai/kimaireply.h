#pragma once

#include <QScopedPointer>
#include <QString>

#include <kemai/kemai_export.h>
#include <kemai/kimaiapi.h>

namespace kemai::client {

class KEMAI_EXPORT KimaiReply
{
public:
    KimaiReply(ApiMethod method, const QByteArray& data);
    virtual ~KimaiReply();

    KimaiReply(const KimaiReply& rhs);
    KimaiReply(KimaiReply&& rhs);

    KimaiReply& operator=(const KimaiReply& rhs);
    KimaiReply& operator=(KimaiReply&& rhs);

    ApiMethod method() const;

    bool isValid() const;

    template<class T> T get() const;

private:
    class KimaiReplyPrivate;
    QScopedPointer<KimaiReplyPrivate> mD;
};

} // namespace kemai::client
