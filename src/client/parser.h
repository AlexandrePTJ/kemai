#pragma once

#include <QJsonObject>
#include <QVariant>

#include "kemai/kimaiapi.h"

namespace kemai::client::parser {

template<typename T> void safeGetJsonValue(const QString& key, const QJsonObject& obj, T& dest)
{
    auto jsval = obj.value(key);
    if (not jsval.isUndefined())
    {
        auto var = jsval.toVariant();
        if (var.canConvert<T>())
        {
            dest = var.value<T>();
        }
    }
}

bool fromJson(const QJsonObject& jso, KimaiVersion& inst);
bool fromJson(const QJsonObject& jso, Customer& inst);
bool fromJson(const QJsonObject& jso, Project& inst);
bool fromJson(const QJsonObject& jso, Activity& inst);
bool fromJson(const QJsonObject& jso, TimeSheet& inst);
bool fromJson(const QJsonObject& jso, User& inst);

QByteArray toPostData(const QJsonValue& jv);
QJsonObject toJson(const TimeSheet& inst);
QJsonObject toJson(const Customer& inst);
QJsonObject toJson(const Project& inst);
QJsonObject toJson(const Activity& inst);

} // namespace kemai::client::parser
