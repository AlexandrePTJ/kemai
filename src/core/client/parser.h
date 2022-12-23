#pragma once

#include <stdexcept>
#include <vector>

#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariant>

#include "kimaiapi.h"

namespace kemai::client::parser {

bool fromJson(const QJsonObject& jso, Customer& inst);

QByteArray toPostData(const QJsonValue& jv);

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

    static QJsonValue toJson(const TimeSheet& inst, TimeSheetConfig::TrackingMode trackingMode);
    static QJsonValue toJson(const Customer& inst);
    static QJsonValue toJson(const Project& inst);
    static QJsonValue toJson(const Activity& inst);

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
