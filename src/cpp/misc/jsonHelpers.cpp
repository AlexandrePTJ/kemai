#include "jsonHelpers.h"

// STL headers
#include <stdexcept>

// fmt headers
#include <fmt/format.h>

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

        file.write(json.toJson());

        if (!file.commit())
        {
            throw std::runtime_error(fmt::format("Cannot write to {}", jsonFilePath));
        }
    }
} // namespace kemai