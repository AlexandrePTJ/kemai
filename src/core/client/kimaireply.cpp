#include "kimaireply.h"

using namespace kemai::client;

void KimaiApiBaseResult::markAsReady()
{
    mIsReady = true;
    emit ready();
}

void KimaiApiBaseResult::setError(const QString &errorMessage)
{
    mError = errorMessage;
    emit error();
}

QString KimaiApiBaseResult::errorMessage() const
{
    return mError.value_or("");
}
