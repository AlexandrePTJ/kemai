#include "datareader.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

using namespace kemai::app;

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

QStringList DataReader::timezones()
{
    QStringList res;

    QFile fdata(":/data/timezones");
    fdata.open(QIODevice::ReadOnly | QIODevice::Text);

    auto jdoc = QJsonDocument::fromJson(fdata.readAll());
    for (const auto& tzVal : jdoc.array())
    {
        res << tzVal.toString();
    }
    return res;
}
