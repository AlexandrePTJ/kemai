#include "parser.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>

namespace kemai::client::parser {

bool fromJson(const QJsonObject& jso, KimaiVersion& inst)
{
    if (not jso.contains("version"))
        return false;

    inst.kimai = jso.value("version").toString();

    return true;
}

bool fromJson(const QJsonObject& jso, Customer& inst)
{
    if (not jso.contains("id"))
        return false;

    inst.name = jso.value("name").toString();
    inst.id   = jso.value("id").toInt();

    return true;
}

bool fromJson(const QJsonObject& jso, Project& inst)
{
    if (not jso.contains("id"))
        return false;

    inst.name = jso.value("name").toString();
    inst.id   = jso.value("id").toInt();

    if (jso.contains("customer"))
        fromJson(jso.value("customer").toObject(), inst.customer);

    return true;
}

bool fromJson(const QJsonObject& jso, Activity& inst)
{
    if (not jso.contains("id"))
        return false;

    inst.name = jso.value("name").toString();
    inst.id   = jso.value("id").toInt();

    return true;
}

bool fromJson(const QJsonObject& jso, TimeSheet& inst)
{
    if (not jso.contains("id"))
        return false;

    fromJson(jso.value("project").toObject(), inst.project);
    fromJson(jso.value("activity").toObject(), inst.activity);

    inst.description = jso.value("description").toString();
    inst.beginAt     = QDateTime::fromString(jso.value("begin").toString(), Qt::ISODate);
    inst.endAt       = QDateTime::fromString(jso.value("end").toString(), Qt::ISODate);
    inst.id          = jso.value("id").toInt();

    return true;
}

bool fromJson(const QJsonObject& jso, User& inst)
{
    if (not jso.contains("id"))
        return false;

    inst.id       = jso.value("id").toInt();
    inst.username = jso.value("username").toString();
    inst.language = jso.value("language").toString();
    inst.timezone = jso.value("timezone").toString();

    return true;
}

QByteArray toPostData(const QJsonValue& jv)
{
    QJsonDocument jdoc;
    if (jv.isArray())
        jdoc = QJsonDocument(jv.toArray());
    else
        jdoc = QJsonDocument(jv.toObject());
    return jdoc.toJson();
}

QJsonObject toJson(const TimeSheet& inst)
{
    QJsonObject joTimeSheet;
    joTimeSheet["begin"] = inst.beginAt.toString(Qt::ISODate);
    if (inst.endAt.isValid())
        joTimeSheet["end"] = inst.endAt.toString(Qt::ISODate);
    joTimeSheet["project"]     = inst.project.id;
    joTimeSheet["activity"]    = inst.activity.id;
    joTimeSheet["description"] = inst.description;
    return joTimeSheet;
}

QJsonObject toJson(const Customer& inst)
{
    QJsonObject joCustomer;

    if (inst.id > 0)
        joCustomer["id"] = inst.id;
    joCustomer["name"]       = inst.name;
    joCustomer["country"]    = inst.countryKey;
    joCustomer["currency"]   = inst.currencyKey;
    joCustomer["timezone"]   = inst.timezone;
    joCustomer["budget"]     = 0;
    joCustomer["timeBudget"] = 0;
    joCustomer["visible"]    = true;

    return joCustomer;
}

QJsonObject toJson(const Project& inst)
{
    QJsonObject joProject;

    joProject["name"]       = inst.name;
    joProject["customer"]   = inst.customer.id;
    joProject["budget"]     = 0;
    joProject["timeBudget"] = 0;
    joProject["visible"]    = true;

    return joProject;
}

QJsonObject toJson(const Activity& inst)
{
    QJsonObject joActivity;

    joActivity["name"] = inst.name;
    if (inst.project)
    {
        joActivity["project"] = inst.project->id;
    }
    joActivity["budget"]     = 0;
    joActivity["timeBudget"] = 0;
    joActivity["visible"]    = true;

    return joActivity;
}

} // namespace kemai::client::parser
