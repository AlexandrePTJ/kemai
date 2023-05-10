#pragma once

#include <stdexcept>
#include <vector>

#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariant>

#include "kimaiAPI.h"

namespace kemai {

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
        return parseValue<T>(m_jsonDocument.object());
    }

    static QJsonValue toJson(const TimeSheet& inst, TimeSheetConfig::TrackingMode trackingMode);
    static QJsonValue toJson(const Customer& inst);
    static QJsonValue toJson(const Project& inst);
    static QJsonValue toJson(const Activity& inst);

private:
    template<class T> T parseValue(const QJsonValue& jsonValue) const;

    template<class T> std::vector<T> parseArrayOf(const QJsonArray& jsonArray) const
    {
        std::vector<T> values;
        for (const auto& jsonValue : jsonArray)
        {
            values.push_back(parseValue<T>(jsonValue));
        }
        return values;
    }

    QJsonDocument m_jsonDocument;
};

} // namespace kemai
