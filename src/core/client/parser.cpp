#include "parser.h"

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

    inst.id = jso.value("id").toInt();
    safeGetJsonValue("name", jso, inst.name);
    safeGetJsonValue("number", jso, inst.number);
    safeGetJsonValue("comment", jso, inst.comment);
    safeGetJsonValue("company", jso, inst.company);
    safeGetJsonValue("address", jso, inst.address);
    safeGetJsonValue("country", jso, inst.countryKey);
    safeGetJsonValue("currency", jso, inst.currencyKey);
    safeGetJsonValue("phone", jso, inst.phone);
    safeGetJsonValue("fax", jso, inst.fax);
    safeGetJsonValue("mobile", jso, inst.mobile);
    safeGetJsonValue("email", jso, inst.email);
    safeGetJsonValue("homepage", jso, inst.homepage);
    safeGetJsonValue("timezone", jso, inst.timezone);
    //    safeGetJsonValue("color", jso, inst.color);
    safeGetJsonValue("budget", jso, inst.budget);
    safeGetJsonValue("timeBudget", jso, inst.timeBudget);
    safeGetJsonValue("visible", jso, inst.visible);

    return true;
}

bool fromJson(const QJsonObject& jso, Project& inst)
{
    if (not jso.contains("id"))
        return false;

    inst.id = jso.value("id").toInt();
    safeGetJsonValue("name", jso, inst.name);
    safeGetJsonValue("comment", jso, inst.comment);
    safeGetJsonValue("orderNumber", jso, inst.orderNumber);
    safeGetJsonValue("orderDate", jso, inst.orderDate);
    safeGetJsonValue("start", jso, inst.start);
    safeGetJsonValue("end", jso, inst.end);
    //    safeGetJsonValue("color", jso, inst.color);
    safeGetJsonValue("budget", jso, inst.budget);
    safeGetJsonValue("timeBudget", jso, inst.timeBudget);
    safeGetJsonValue("visible", jso, inst.visible);

    if (jso.contains("customer"))
    {
        const auto& jsvCustomer = jso.value("customer");
        if (jsvCustomer.isObject())
            fromJson(jsvCustomer.toObject(), inst.customer);
        else if (jsvCustomer.isDouble())
            inst.customer.id = jsvCustomer.toInt();
    }

    return true;
}

bool fromJson(const QJsonObject& jso, Activity& inst)
{
    if (not jso.contains("id"))
        return false;

    inst.id = jso.value("id").toInt();
    safeGetJsonValue("name", jso, inst.name);
    safeGetJsonValue("comment", jso, inst.comment);
    //    safeGetJsonValue("color", jso, inst.color);
    safeGetJsonValue("budget", jso, inst.budget);
    safeGetJsonValue("timeBudget", jso, inst.timeBudget);
    safeGetJsonValue("visible", jso, inst.visible);

    // handle project-less project
    if (jso.contains("project"))
    {
        if (!jso.value("project").isNull())
        {
            Project project;
            project.id   = jso.value("project").toInt();
            inst.project = project;
        }
    }

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

    for (const auto& jstag : jso.value("tags").toArray())
        inst.tags << jstag.toString();

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
    joTimeSheet["tags"]        = inst.tags.join(',');
    return joTimeSheet;
}

QJsonObject toJson(const Customer& inst)
{
    QJsonObject joCustomer;

    if (inst.id > 0)
        joCustomer["id"] = inst.id;

    joCustomer["name"]     = inst.name;
    joCustomer["number"]   = inst.number;
    joCustomer["comment"]  = inst.comment;
    joCustomer["company"]  = inst.company;
    joCustomer["address"]  = inst.address;
    joCustomer["country"]  = inst.countryKey;
    joCustomer["currency"] = inst.currencyKey;
    joCustomer["phone"]    = inst.phone;
    joCustomer["fax"]      = inst.fax;
    joCustomer["mobile"]   = inst.mobile;
    joCustomer["email"]    = inst.email;
    joCustomer["homepage"] = inst.homepage;
    joCustomer["timezone"] = inst.timezone;
    //    joCustomer["color"]       = inst.color;
    joCustomer["budget"]     = inst.budget;
    joCustomer["timeBudget"] = inst.timeBudget;
    joCustomer["visible"]    = inst.visible;

    return joCustomer;
}

QJsonObject toJson(const Project& inst)
{
    QJsonObject joProject;

    if (inst.id > 0)
        joProject["id"] = inst.id;

    joProject["name"]        = inst.name;
    joProject["visible"]     = inst.visible;
    joProject["comment"]     = inst.comment;
    joProject["orderNumber"] = inst.orderNumber;
    joProject["orderDate"]   = inst.orderDate;
    joProject["start"]       = inst.start;
    joProject["end"]         = inst.end;
    joProject["color"]       = inst.color;
    joProject["customer"]    = inst.customer.id;
    joProject["budget"]      = inst.budget;
    joProject["timeBudget"]  = inst.timeBudget;

    return joProject;
}

QJsonObject toJson(const Activity& inst)
{
    QJsonObject joActivity;

    joActivity["name"]    = inst.name;
    joActivity["visible"] = inst.visible;
    joActivity["comment"] = inst.comment;
    if (inst.project)
    {
        joActivity["project"] = inst.project->id;
    }
    //    joCustomer["color"]       = inst.color;
    joActivity["budget"]     = inst.budget;
    joActivity["timeBudget"] = inst.timeBudget;

    return joActivity;
}

} // namespace kemai::client::parser
