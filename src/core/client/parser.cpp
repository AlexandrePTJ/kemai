#include "parser.h"

#include <QJsonArray>
#include <QJsonDocument>

namespace kemai::client::parser {

TimeSheetConfig::TrackingMode trackingModeFromString(const QString& trackingMode)
{
    if (trackingMode == "default")
    {
        return TimeSheetConfig::TrackingMode::Default;
    }
    if (trackingMode == "duration_fixed_begin")
    {
        return TimeSheetConfig::TrackingMode::DurationFixedBegin;
    }
    if (trackingMode == "duration_only")
    {
        return TimeSheetConfig::TrackingMode::DurationOnly;
    }
    if (trackingMode == "punch")
    {
        return TimeSheetConfig::TrackingMode::Punch;
    }
    return TimeSheetConfig::TrackingMode::Default;
}

Task::Status taskStatusFromString(const QString& taskStatus)
{
    if (taskStatus == "pending")
    {
        return Task::Status::Pending;
    }
    if (taskStatus == "progress")
    {
        return Task::Status::Progress;
    }
    if (taskStatus == "closed")
    {
        return Task::Status::Closed;
    }
    return Task::Status::Undefined;
}

bool fromJson(const QJsonObject& jso, KimaiVersion& inst)
{
    if (!jso.contains("version"))
    {
        return false;
    }

    inst.kimai = QVersionNumber::fromString(jso.value("version").toString());

    return true;
}

bool fromJson(const QJsonObject& jso, Customer& inst)
{
    if (!jso.contains("id"))
    {
        return false;
    }

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
    if (!jso.contains("id"))
    {
        return false;
    }

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
        {
            fromJson(jsvCustomer.toObject(), inst.customer);
        }
        else if (jsvCustomer.isDouble())
        {
            inst.customer.id = jsvCustomer.toInt();
        }
    }

    return true;
}

bool fromJson(const QJsonObject& jso, Activity& inst)
{
    if (!jso.contains("id"))
    {
        return false;
    }

    inst.id = jso.value("id").toInt();
    safeGetJsonValue("name", jso, inst.name);
    safeGetJsonValue("comment", jso, inst.comment);
    //    safeGetJsonValue("color", jso, inst.color);
    safeGetJsonValue("budget", jso, inst.budget);
    safeGetJsonValue("timeBudget", jso, inst.timeBudget);
    safeGetJsonValue("visible", jso, inst.visible);

    // handle project-less activity
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
    if (!jso.contains("id"))
    {
        return false;
    }

    fromJson(jso.value("project").toObject(), inst.project);
    fromJson(jso.value("activity").toObject(), inst.activity);

    inst.description = jso.value("description").toString();
    inst.beginAt     = QDateTime::fromString(jso.value("begin").toString(), Qt::ISODate);
    inst.endAt       = QDateTime::fromString(jso.value("end").toString(), Qt::ISODate);
    inst.id          = jso.value("id").toInt();
    inst.user        = jso.value("user").toInt();

    for (const auto& jstag : jso.value("tags").toArray())
    {
        inst.tags << jstag.toString();
    }

    return true;
}

bool fromJson(const QJsonObject& jso, User& inst)
{
    if (!jso.contains("id"))
    {
        return false;
    }

    inst.id       = jso.value("id").toInt();
    inst.username = jso.value("username").toString();
    inst.language = jso.value("language").toString();
    inst.timezone = jso.value("timezone").toString();

    return true;
}

bool fromJson(const QJsonObject& jso, Task& inst)
{
    if (!jso.contains("id"))
    {
        return false;
    }

    inst.id     = jso.value("id").toInt();
    inst.title  = jso.value("title").toString();
    inst.status = taskStatusFromString(jso.value("status").toString());
    safeGetJsonValue("todo", jso, inst.todo);
    safeGetJsonValue("description", jso, inst.description);
    fromJson(jso.value("project").toObject(), inst.project);
    fromJson(jso.value("activity").toObject(), inst.activity);
    fromJson(jso.value("user").toObject(), inst.user);
    inst.endAt = QDateTime::fromString(jso.value("end").toString(), Qt::ISODate);
    safeGetJsonValue("estimation", jso, inst.estimation);

    if (jso.contains("activeTimesheets"))
    {
        const auto& jaTimesheets = jso.value("activeTimesheets").toArray();
        for (const auto& jvTimesheet : jaTimesheets)
        {
            TimeSheet ts;
            if (fromJson(jvTimesheet.toObject(), ts))
            {
                inst.activeTimeSheets << ts;
            }
        }
    }

    return true;
}

bool fromJson(const QJsonObject& jso, Plugin& inst)
{
    if (!jso.contains("name") || !jso.contains("version"))
    {
        return false;
    }

    inst.name      = jso.value("name").toString();
    inst.version   = QVersionNumber::fromString(jso.value("version").toString());
    inst.apiPlugin = pluginByName(inst.name);
    return true;
}

bool fromJson(const QJsonObject& jso, TimeSheetConfig& inst)
{
    if (!jso.contains("trackingMode"))
    {
        return false;
    }

    inst.trackingMode = trackingModeFromString(jso.value("trackingMode").toString());
    return true;
}

QByteArray toPostData(const QJsonValue& jv)
{
    QJsonDocument jdoc;
    if (jv.isArray())
    {
        jdoc = QJsonDocument(jv.toArray());
    }
    else
    {
        jdoc = QJsonDocument(jv.toObject());
    }
    return jdoc.toJson(QJsonDocument::Compact);
}

QJsonObject toJson(const TimeSheet& inst, TimeSheetConfig::TrackingMode trackingMode)
{
    QJsonObject joTimeSheet;
    if (trackingMode != TimeSheetConfig::TrackingMode::Punch)
    {
        joTimeSheet["begin"] = inst.beginAt.toString(Qt::ISODate);
        if (inst.endAt.isValid())
        {
            joTimeSheet["end"] = inst.endAt.toString(Qt::ISODate);
        }
    }
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
    {
        joCustomer["id"] = inst.id;
    }

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
    {
        joProject["id"] = inst.id;
    }

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
