// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "jsonHelpers.h"

// STL headers
#include <stdexcept>

// 3rd party headers
#include <fmt/format.h>
#include <magic_enum/magic_enum.hpp>

// Qt headers
#include <QFile>
#include <QJsonDocument>
#include <QSaveFile>

// Project headers
#include <misc/customFmt.h>

namespace
{
    QJsonDocument loadJsonFromFile(const QString &jsonFilePath)
    {
        if (!QFile::exists(jsonFilePath))
        {
            throw std::runtime_error(fmt::format("{} does not exists", jsonFilePath));
        }

        QFile file(jsonFilePath);
        if (!file.open(QFile::ReadOnly))
        {
            throw std::runtime_error(fmt::format("Cannot open {}", jsonFilePath));
        }

        QJsonParseError error;
        auto            doc = QJsonDocument::fromJson(file.readAll(), &error);

        if (error.error != QJsonParseError::NoError)
        {
            throw std::runtime_error(fmt::format("Cannot parse {}: {}", jsonFilePath, error.errorString()));
        }

        return doc;
    }
} // namespace

namespace kemai
{
    QJsonArray JsonHelpers::loadJsonArrayFromFile(const QString &jsonFilePath)
    {
        auto doc = loadJsonFromFile(jsonFilePath);
        if (!doc.isArray())
        {
            return {};
        }
        return doc.array();
    }

    QJsonObject JsonHelpers::loadJsonObjectFromFile(const QString &jsonFilePath)
    {
        auto doc = loadJsonFromFile(jsonFilePath);
        if (!doc.isObject())
        {
            return {};
        }
        return doc.object();
    }

    void JsonHelpers::saveJsonToFile(const QJsonValue &json, const QString &jsonFilePath)
    {
        QSaveFile file(jsonFilePath);
        if (!file.open(QFile::WriteOnly))
        {
            throw std::runtime_error(fmt::format("Cannot open {}", jsonFilePath));
        }

        file.write(toByteArray(json, QJsonDocument::Indented));

        if (!file.commit())
        {
            throw std::runtime_error(fmt::format("Cannot write to {}", jsonFilePath));
        }
    }

    QByteArray JsonHelpers::toByteArray(const QJsonValue &json, QJsonDocument::JsonFormat format)
    {
        QJsonDocument doc;
        if (json.isObject())
        {
            doc = QJsonDocument(json.toObject());
        }
        else if (json.isArray())
        {
            doc = QJsonDocument(json.toArray());
        }
        return doc.toJson(format);
    }

    void JsonHelpers::checkKeysOrThrow(const QString &objectName, const QJsonObject &jsonObject, const QStringList &requiredKeys)
    {
        for (const auto &key : requiredKeys)
        {
            if (!jsonObject.contains(key))
            {
                throw std::runtime_error(fmt::format("Invalid {} object. Key '{}' is missing", objectName, key));
            }
        }
    }

    void JsonHelpers::checkTypeOrThrow(const QString &objectName, const QJsonValue &jsonValue, const std::vector<QJsonValue::Type> &allowedTypes)
    {
        if (std::ranges::none_of(allowedTypes, [jsonValue](QJsonValue::Type type)
                                 { return type == jsonValue.type(); }))
        {
            throw std::runtime_error(fmt::format("Invalid type for {}: {}", objectName, magic_enum::enum_name(jsonValue.type())));
        }
    }
} // namespace kemai