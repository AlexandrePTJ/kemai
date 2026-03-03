#pragma once

// Qt headers
#include <QJsonArray>
#include <QJsonObject>

namespace kemai
{

    class JsonHelpers
    {
    public:
        /*!
         *
         * @param jsonFilePath
         * @return
         * @throw std::runtime_error When cannot read document or is not valid json
         */
        static QJsonArray loadJsonArrayFromFile(const QString &jsonFilePath);

        /*!
         *
         * @param jsonFilePath
         * @return
         * @throw std::runtime_error When cannot read document or is not valid json
         */
        static QJsonObject loadJsonObjectFromFile(const QString &jsonFilePath);

        /*!
         *
         * @param json
         * @param jsonFilePath
         * @throw std::runtime_error When cannot write document
         */
        static void saveJsonToFile(const QJsonValue &json, const QString &jsonFilePath);
    };
} // namespace kemai