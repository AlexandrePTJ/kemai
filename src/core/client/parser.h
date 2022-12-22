#pragma once

#include <stdexcept>

#include <QJsonObject>
#include <QVariant>
#include <QJsonArray>
#include <QJsonDocument>

#include "kimaiapi.h"

namespace kemai::client::parser {

template<typename T> void safeGetJsonValue(const QString& key, const QJsonObject& obj, T& dest)
{
    auto jsval = obj.value(key);
    if (!jsval.isUndefined())
    {
        auto var = jsval.toVariant();
        if (var.canConvert<T>())
        {
            dest = var.value<T>();
        }
    }
}

bool fromJson(const QJsonObject& jso, Customer& inst);
bool fromJson(const QJsonObject& jso, Project& inst);
bool fromJson(const QJsonObject& jso, Activity& inst);
bool fromJson(const QJsonObject& jso, TimeSheet& inst);
bool fromJson(const QJsonObject& jso, User& inst);
bool fromJson(const QJsonObject& jso, Task& inst);

QByteArray toPostData(const QJsonValue& jv);
QJsonObject toJson(const TimeSheet& inst, TimeSheetConfig::TrackingMode trackingMode);
QJsonObject toJson(const Customer& inst);
QJsonObject toJson(const Project& inst);
QJsonObject toJson(const Activity& inst);

} // namespace kemai::client::parser

namespace kemai::client {

class KimaiApiTypesParser
{
public:
    /*!
     *
     * \throw std::runtime_error if data is not json
     */
    KimaiApiTypesParser(const QByteArray& data);

   template<class T> std::vector<T> getArrayOf() const
    {
        if (!m_jsonDocument.isArray())
        {
            throw std::runtime_error("JSON value is not an array");
        }
        return parseArrayOf<T>(m_jsonDocument.array());
    }

    template<class T> T getValueOf() const
    {
        if (!m_jsonDocument.isObject())
        {
            throw std::runtime_error("JSON value is not an object");
        }
        return parseObject<T>(m_jsonDocument.object());
    }

private:

    template<class T> T parseObject(const QJsonObject& jsonObject) const;

    template<class T> std::vector<T> parseArrayOf(const QJsonArray& jsonArray) const
    {
        std::vector<T> values;
        for (const auto& jsonValue : jsonArray)
        {
            if (!jsonValue.isObject())
            {
                throw std::runtime_error("Unexpected json value type");
            }
            values.push_back(parseObject<T>(jsonValue.toObject()));
        }
        return values;
    }

    QJsonDocument m_jsonDocument;
};

} // namespace kemai::client
