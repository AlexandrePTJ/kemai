#include "dataReader.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

using namespace kemai;

static QMap<QString, QString> readKeyValueJsonFile(const QString& jsonFile)
{
    QMap<QString, QString> res;

    QFile dataFile(jsonFile);
    dataFile.open(QIODevice::ReadOnly | QIODevice::Text);

    auto jsonDocument = QJsonDocument::fromJson(dataFile.readAll());
    auto jsonObject   = jsonDocument.object();
    for (auto it = jsonObject.begin(); it != jsonObject.end(); ++it)
    {
        res.insert(it.key(), it.value().toString());
    }

    return res;
}

QMap<QString, QString> DataReader::countries()
{
    return readKeyValueJsonFile(":/data/countries");
}

QMap<QString, QString> DataReader::currencies()
{
    return readKeyValueJsonFile(":/data/currencies");
}
