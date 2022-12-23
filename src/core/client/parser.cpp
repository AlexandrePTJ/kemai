#include "parser.h"

#include <QJsonArray>
#include <QJsonDocument>

using namespace kemai::client;

static TimeSheetConfig::TrackingMode trackingModeFromString(const QString& trackingMode)
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

static Task::Status taskStatusFromString(const QString& taskStatus)
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

template<typename T> void safeGetJsonValue(const QString& key, const QJsonObject& obj, T& dest)
{
    auto jsval = obj.value(key);
    if (!jsval.isUndefined())
    {
        auto var = jsval.toVariant();
        if (var.canConvert<T>())
        {
            dest = var.value<T>();
        }
    }
}

namespace kemai::client::parser {

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


} // namespace kemai::client::parser

static void checkKeysOrThrow(const QString& objectName, const QJsonObject& jsonObject, const QStringList& requiredKeys)
{
    for (const auto& key : requiredKeys)
    {
        if (!jsonObject.contains(key))
        {
            throw std::runtime_error(QString("Invalid %1 object. Key '%2' is missing").arg(objectName, key).toStdString());
        }
    }
}

KimaiApiTypesParser::KimaiApiTypesParser(const QByteArray& data)
{
    QJsonParseError parseError;
    m_jsonDocument = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        throw std::runtime_error(QString("Data is not a valid json: %1").arg(parseError.errorString()).toStdString());
    }
}

template<> KimaiVersion KimaiApiTypesParser::parseObject(const QJsonObject& jsonObject) const
{
    checkKeysOrThrow("KimaiVersion", jsonObject, {"version"});

    KimaiVersion kimaiVersion;
    kimaiVersion.kimai = QVersionNumber::fromString(jsonObject.value("version").toString());

    return kimaiVersion;
}

template<> User KimaiApiTypesParser::parseObject(const QJsonObject& jsonObject) const
{
    checkKeysOrThrow("User", jsonObject, {"id"});

    User user;
    user.id       = jsonObject.value("id").toInt();
    user.username = jsonObject.value("username").toString();
    user.language = jsonObject.value("language").toString();
    user.timezone = jsonObject.value("timezone").toString();

    return user;
}

template<> TimeSheetConfig KimaiApiTypesParser::parseObject(const QJsonObject& jsonObject) const
{
    checkKeysOrThrow("TimeSheetConfig", jsonObject, {"trackingMode"});

    TimeSheetConfig timeSheetConfig;
    timeSheetConfig.trackingMode = trackingModeFromString(jsonObject.value("trackingMode").toString());

    return timeSheetConfig;
}

template<> Plugin KimaiApiTypesParser::parseObject(const QJsonObject& jsonObject) const
{
    checkKeysOrThrow("Plugin", jsonObject, {"name", "version"});

    Plugin plugin;
    plugin.name      = jsonObject.value("name").toString();
    plugin.version   = QVersionNumber::fromString(jsonObject.value("version").toString());
    plugin.apiPlugin = pluginByName(plugin.name);

    return plugin;
}

template<> Customer KimaiApiTypesParser::parseObject(const QJsonObject& jsonObject) const
{
    checkKeysOrThrow("Customer", jsonObject, {"id"});

    Customer customer;
    customer.id = jsonObject.value("id").toInt();
    safeGetJsonValue("name", jsonObject, customer.name);
    safeGetJsonValue("number", jsonObject, customer.number);
    safeGetJsonValue("comment", jsonObject, customer.comment);
    safeGetJsonValue("company", jsonObject, customer.company);
    safeGetJsonValue("address", jsonObject, customer.address);
    safeGetJsonValue("country", jsonObject, customer.countryKey);
    safeGetJsonValue("currency", jsonObject, customer.currencyKey);
    safeGetJsonValue("phone", jsonObject, customer.phone);
    safeGetJsonValue("fax", jsonObject, customer.fax);
    safeGetJsonValue("mobile", jsonObject, customer.mobile);
    safeGetJsonValue("email", jsonObject, customer.email);
    safeGetJsonValue("homepage", jsonObject, customer.homepage);
    safeGetJsonValue("timezone", jsonObject, customer.timezone);
    //    parser::safeGetJsonValue("jsonObject", jsonObject, customer.color);
    safeGetJsonValue("budget", jsonObject, customer.budget);
    safeGetJsonValue("timeBudget", jsonObject, customer.timeBudget);
    safeGetJsonValue("visible", jsonObject, customer.visible);

    return customer;
}

template<> Project KimaiApiTypesParser::parseObject(const QJsonObject& jsonObject) const
{
    checkKeysOrThrow("Project", jsonObject, {"id"});

    Project project;
    project.id = jsonObject.value("id").toInt();
    safeGetJsonValue("name", jsonObject, project.name);
    safeGetJsonValue("comment", jsonObject, project.comment);
    safeGetJsonValue("orderNumber", jsonObject, project.orderNumber);
    safeGetJsonValue("orderDate", jsonObject, project.orderDate);
    safeGetJsonValue("start", jsonObject, project.start);
    safeGetJsonValue("end", jsonObject, project.end);
    //    safeGetJsonValue("jsonObject", jsonObject, project.color);
    safeGetJsonValue("budget", jsonObject, project.budget);
    safeGetJsonValue("timeBudget", jsonObject, project.timeBudget);
    safeGetJsonValue("visible", jsonObject, project.visible);

    if (jsonObject.contains("customer"))
    {
        const auto& jsvCustomer = jsonObject.value("customer");
        if (jsvCustomer.isObject())
        {
            project.customer = parseObject<Customer>(jsvCustomer.toObject());
        }
        else if (jsvCustomer.isDouble())
        {
            project.customer.id = jsvCustomer.toInt();
        }
    }

    return project;
}

template<> Activity KimaiApiTypesParser::parseObject(const QJsonObject& jsonObject) const
{
    checkKeysOrThrow("Activity", jsonObject, {"id"});

    Activity activity;
    activity.id = jsonObject.value("id").toInt();
    safeGetJsonValue("name", jsonObject, activity.name);
    safeGetJsonValue("comment", jsonObject, activity.comment);
    //    safeGetJsonValue("color", jsonObject, activity.color);
    safeGetJsonValue("budget", jsonObject, activity.budget);
    safeGetJsonValue("timeBudget", jsonObject, activity.timeBudget);
    safeGetJsonValue("visible", jsonObject, activity.visible);

    // handle project-less activity
    if (jsonObject.contains("project"))
    {
        if (!jsonObject.value("project").isNull())
        {
            Project project;
            project.id       = jsonObject.value("project").toInt();
            activity.project = project;
        }
    }

    return activity;
}

template<> TimeSheet KimaiApiTypesParser::parseObject(const QJsonObject& jsonObject) const
{
    checkKeysOrThrow("TimeSheet", jsonObject, {"id"});

    TimeSheet timeSheet;

    timeSheet.project     = parseObject<Project>(jsonObject.value("project").toObject());
    timeSheet.activity    = parseObject<Activity>(jsonObject.value("activity").toObject());
    timeSheet.description = jsonObject.value("description").toString();
    timeSheet.beginAt     = QDateTime::fromString(jsonObject.value("begin").toString(), Qt::ISODate);
    timeSheet.endAt       = QDateTime::fromString(jsonObject.value("end").toString(), Qt::ISODate);
    timeSheet.id          = jsonObject.value("id").toInt();
    timeSheet.user        = jsonObject.value("user").toInt();

    for (const auto& jsTag : jsonObject.value("tags").toArray())
    {
        timeSheet.tags.push_back(jsTag.toString());
    }

    return timeSheet;
}

template<> Task KimaiApiTypesParser::parseObject(const QJsonObject& jsonObject) const
{
    checkKeysOrThrow("Task", jsonObject, {"id"});

    Task task;
    task.id     = jsonObject.value("id").toInt();
    task.title  = jsonObject.value("title").toString();
    task.status = taskStatusFromString(jsonObject.value("status").toString());
    safeGetJsonValue("todo", jsonObject, task.todo);
    safeGetJsonValue("description", jsonObject, task.description);
    task.project  = parseObject<Project>(jsonObject.value("project").toObject());
    task.activity = parseObject<Activity>(jsonObject.value("activity").toObject());
    task.user     = parseObject<User>(jsonObject.value("user").toObject());
    task.endAt    = QDateTime::fromString(jsonObject.value("end").toString(), Qt::ISODate);
    safeGetJsonValue("estimation", jsonObject, task.estimation);

    if (jsonObject.contains("activeTimesheets"))
    {
        const auto& jaTimeSheets = jsonObject.value("activeTimesheets").toArray();
        for (const auto& jvTimeSheet : jaTimeSheets)
        {
            task.activeTimeSheets.push_back(parseObject<TimeSheet>(jvTimeSheet.toObject()));
        }
    }

    return task;
}

QJsonValue KimaiApiTypesParser::toJson(const TimeSheet& inst, TimeSheetConfig::TrackingMode trackingMode)
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

QJsonValue KimaiApiTypesParser::toJson(const Customer& inst)
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

QJsonValue KimaiApiTypesParser::toJson(const Project& inst)
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

QJsonValue KimaiApiTypesParser::toJson(const Activity& inst)
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
