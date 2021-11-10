#pragma once

#include <QScopedPointer>
#include <QString>

#include "kimaiapi.h"

namespace kemai::client {

class KimaiReply
{
public:
    KimaiReply(ApiMethod method, const QByteArray& data);
    virtual ~KimaiReply();

    KimaiReply(const KimaiReply& rhs);
    KimaiReply(KimaiReply&& rhs) noexcept;

    KimaiReply& operator=(const KimaiReply& rhs);
    KimaiReply& operator=(KimaiReply&& rhs) noexcept;

    ApiMethod method() const;

    bool isValid() const;

    template<class T> T get() const;

private:
    class KimaiReplyPrivate;
    QScopedPointer<KimaiReplyPrivate> mD;
};

} // namespace kemai::client
