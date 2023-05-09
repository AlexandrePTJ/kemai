#include "dataReader.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

using namespace kemai;

QMap<QString, QString> DataReader::countries()
{
    QMap<QString, QString> res;

    QFile fdata(":/data/countries");
    fdata.open(QIODevice::ReadOnly | QIODevice::Text);

    auto jdoc = QJsonDocument::fromJson(fdata.readAll());
    auto jobj = jdoc.object();
    for (auto it = jobj.begin(); it != jobj.end(); ++it)
    {
        res.insert(it.key(), it.value().toString());
    }

    return res;
}

QMap<QString, QString> DataReader::currencies()
{
    QMap<QString, QString> res;

    QFile fdata(":/data/currencies");
    fdata.open(QIODevice::ReadOnly | QIODevice::Text);

    auto jdoc = QJsonDocument::fromJson(fdata.readAll());
    auto jobj = jdoc.object();
    for (auto it = jobj.begin(); it != jobj.end(); ++it)
    {
        res.insert(it.key(), it.value().toString());
    }

    return res;
}
