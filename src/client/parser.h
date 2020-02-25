#pragma once

#include <QJsonObject>

#include "kemai/kimaiapi.h"

namespace kemai::client::parser {

bool fromJson(const QJsonObject& jso, KimaiVersion& inst);
bool fromJson(const QJsonObject& jso, Customer& inst);
bool fromJson(const QJsonObject& jso, Project& inst);
bool fromJson(const QJsonObject& jso, Activity& inst);
bool fromJson(const QJsonObject& jso, TimeSheet& inst);

QByteArray toPostData(const QJsonValue& jv);
QJsonObject toJson(const TimeSheet& inst);
QJsonObject toJson(const Customer& inst);

} // namespace kemai::client::parser
