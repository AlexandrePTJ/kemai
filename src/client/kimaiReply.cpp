#include "kimaiReply.h"

#include <spdlog/spdlog.h>

using namespace kemai;

void KimaiApiBaseResult::markAsReady()
{
    mIsReady = true;
    emit ready();
}

bool KimaiApiBaseResult::hasError() const
{
    return mIsReady && mError.has_value();
}

void KimaiApiBaseResult::setError(const QString& errorMessage)
{
    mIsReady = true;
    mError   = errorMessage;
    spdlog::error("<=== {}", errorMessage.toStdString());
    emit error();
}

QString KimaiApiBaseResult::errorMessage() const
{
    return mError.value_or("");
}
