#include "parser.h"

#include <QJsonArray>
#include <QJsonDocument>

#include <magic_enum.hpp>

using namespace kemai;

/*
 * Static helpers
 */
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

static void checkTypeOrThrow(const QString& objectName, const QJsonValue& jsonValue, const std::vector<QJsonValue::Type>& allowedTypes)
{
    if (std::none_of(allowedTypes.begin(), allowedTypes.end(), [jsonValue](QJsonValue::Type type) { return type == jsonValue.type(); }))
    {
        throw std::runtime_error(
            QString("Invalid type for %1: %2").arg(objectName, QString::fromStdString(std::string(magic_enum::enum_name(jsonValue.type())))).toStdString());
    }
}

/*
 * KimaiApiTypesParser implementation
 */
KimaiApiTypesParser::KimaiApiTypesParser(const QByteArray& data)
{
    QJsonParseError parseError;
    m_jsonDocument = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        throw std::runtime_error(QString("Data is not a valid json: %1").arg(parseError.errorString()).toStdString());
    }
}

template<> KimaiVersion KimaiApiTypesParser::parseValue(const QJsonValue& jsonValue) const
{
    checkTypeOrThrow("KimaiVersion", jsonValue, {QJsonValue::Object});

    auto jsonObject = jsonValue.toObject();
    checkKeysOrThrow("KimaiVersion", jsonObject, {"version"});

    KimaiVersion kimaiVersion;
    kimaiVersion.kimai = QVersionNumber::fromString(jsonObject.value("version").toString());

    return kimaiVersion;
}

template<> User KimaiApiTypesParser::parseValue(const QJsonValue& jsonValue) const
{
    checkTypeOrThrow("User", jsonValue, {QJsonValue::Object, QJsonValue::Double});

    if (jsonValue.isDouble())
    {
        User user;
        user.id = jsonValue.toInt();
        return user;
    }

    auto jsonObject = jsonValue.toObject();
    checkKeysOrThrow("User", jsonObject, {"id", "username", "memberships"});

    User user;
    user.id       = jsonObject.value("id").toInt();
    user.username = jsonObject.value("username").toString();
    user.language = jsonObject.value("language").toString();
    user.timezone = jsonObject.value("timezone").toString();

    return user;
}

template<> TimeSheetConfig KimaiApiTypesParser::parseValue(const QJsonValue& jsonValue) const
{
    checkTypeOrThrow("TimeSheetConfig", jsonValue, {QJsonValue::Object});

    auto jsonObject = jsonValue.toObject();
    checkKeysOrThrow("TimeSheetConfig", jsonObject, {"trackingMode"});

    TimeSheetConfig timeSheetConfig;
    timeSheetConfig.trackingMode = trackingModeFromString(jsonObject.value("trackingMode").toString());

    return timeSheetConfig;
}

template<> Plugin KimaiApiTypesParser::parseValue(const QJsonValue& jsonValue) const
{
    checkTypeOrThrow("Plugin", jsonValue, {QJsonValue::Object});

    auto jsonObject = jsonValue.toObject();
    checkKeysOrThrow("Plugin", jsonObject, {"name", "version"});

    Plugin plugin;
    plugin.name      = jsonObject.value("name").toString();
    plugin.version   = QVersionNumber::fromString(jsonObject.value("version").toString());
    plugin.apiPlugin = pluginByName(plugin.name);

    return plugin;
}

template<> Customer KimaiApiTypesParser::parseValue(const QJsonValue& jsonValue) const
{
    checkTypeOrThrow("Customer", jsonValue, {QJsonValue::Object, QJsonValue::Double});

    if (jsonValue.isDouble())
    {
        Customer customer;
        customer.id = jsonValue.toInt();
        return customer;
    }

    auto jsonObject = jsonValue.toObject();
    checkKeysOrThrow("Customer", jsonObject, {"id"});

    Customer customer;
    customer.id          = jsonObject.value("id").toInt();
    customer.name        = jsonObject.value("name").toString();
    customer.number      = jsonObject.value("number").toString();
    customer.comment     = jsonObject.value("comment").toString();
    customer.company     = jsonObject.value("company").toString();
    customer.address     = jsonObject.value("address").toString();
    customer.countryKey  = jsonObject.value("country").toString();
    customer.currencyKey = jsonObject.value("currency").toString();
    customer.phone       = jsonObject.value("phone").toString();
    customer.fax         = jsonObject.value("fax").toString();
    customer.mobile      = jsonObject.value("mobile").toString();
    customer.email       = jsonObject.value("email").toString();
    customer.homepage    = jsonObject.value("homepage").toString();
    customer.timezone    = jsonObject.value("timezone").toString();
    customer.color       = jsonObject.value("color").toString();
    customer.budget      = jsonObject.value("budget").toDouble();
    customer.timeBudget  = jsonObject.value("timeBudget").toInt();
    customer.visible     = jsonObject.value("visible").toBool();

    return customer;
}

template<> Project KimaiApiTypesParser::parseValue(const QJsonValue& jsonValue) const
{
    checkTypeOrThrow("Project", jsonValue, {QJsonValue::Object, QJsonValue::Double});

    if (jsonValue.isDouble())
    {
        Project project;
        project.id = jsonValue.toInt();
        return project;
    }

    auto jsonObject = jsonValue.toObject();
    checkKeysOrThrow("Project", jsonObject, {"id", "name"});

    Project project;
    project.id          = jsonObject.value("id").toInt();
    project.name        = jsonObject.value("name").toString();
    project.comment     = jsonObject.value("comment").toString();
    project.orderNumber = jsonObject.value("orderNumber").toString();
    project.orderDate   = jsonObject.value("orderDate").toString();
    project.start       = jsonObject.value("start").toString();
    project.end         = jsonObject.value("end").toString();
    project.color       = jsonObject.value("color").toString();
    project.budget      = jsonObject.value("budget").toDouble();
    project.timeBudget  = jsonObject.value("timeBudget").toInt();
    project.visible     = jsonObject.value("visible").toBool();

    if (jsonObject.contains("customer"))
    {
        const auto& jsvCustomer = jsonObject.value("customer");
        project.customer        = parseValue<Customer>(jsvCustomer);
    }

    return project;
}

template<> Activity KimaiApiTypesParser::parseValue(const QJsonValue& jsonValue) const
{
    checkTypeOrThrow("Activity", jsonValue, {QJsonValue::Object, QJsonValue::Double});

    if (jsonValue.isDouble())
    {
        Activity activity;
        activity.id = jsonValue.toInt();
        return activity;
    }

    auto jsonObject = jsonValue.toObject();
    checkKeysOrThrow("Activity", jsonObject, {"id", "name"});

    Activity activity;
    activity.id         = jsonObject.value("id").toInt();
    activity.name       = jsonObject.value("name").toString();
    activity.comment    = jsonObject.value("comment").toString();
    activity.color      = jsonObject.value("color").toString();
    activity.budget     = jsonObject.value("budget").toDouble();
    activity.timeBudget = jsonObject.value("timeBudget").toInt();
    activity.visible    = jsonObject.value("visible").toBool();

    // handle project-less activity
    if (jsonObject.contains("project"))
    {
        if (!jsonObject.value("project").isNull())
        {
            activity.project = parseValue<Project>(jsonObject.value("project"));
        }
    }

    return activity;
}

template<> TimeSheet KimaiApiTypesParser::parseValue(const QJsonValue& jsonValue) const
{
    checkTypeOrThrow("TimeSheet", jsonValue, {QJsonValue::Object, QJsonValue::Double});

    if (jsonValue.isDouble())
    {
        TimeSheet timeSheet;
        timeSheet.id = jsonValue.toInt();
        return timeSheet;
    }

    auto jsonObject = jsonValue.toObject();
    checkKeysOrThrow("TimeSheet", jsonObject, {"id", "begin"});

    TimeSheet timeSheet;
    timeSheet.id          = jsonObject.value("id").toInt();
    timeSheet.project     = parseValue<Project>(jsonObject.value("project"));
    timeSheet.activity    = parseValue<Activity>(jsonObject.value("activity"));
    timeSheet.description = jsonObject.value("description").toString();
    timeSheet.beginAt     = QDateTime::fromString(jsonObject.value("begin").toString(), Qt::ISODate);
    timeSheet.endAt       = QDateTime::fromString(jsonObject.value("end").toString(), Qt::ISODate);
    timeSheet.user        = jsonObject.value("user").toInt();

    for (const auto& jsTag : jsonObject.value("tags").toArray())
    {
        timeSheet.tags.push_back(jsTag.toString());
    }

    return timeSheet;
}

template<> Task KimaiApiTypesParser::parseValue(const QJsonValue& jsonValue) const
{
    checkTypeOrThrow("Task", jsonValue, {QJsonValue::Object, QJsonValue::Double});

    if (jsonValue.isDouble())
    {
        Task task;
        task.id = jsonValue.toInt();
        return task;
    }

    auto jsonObject = jsonValue.toObject();
    checkKeysOrThrow("Task", jsonObject, {"id", "title"});

    Task task;
    task.id          = jsonObject.value("id").toInt();
    task.title       = jsonObject.value("title").toString();
    task.status      = taskStatusFromString(jsonObject.value("status").toString());
    task.todo        = jsonObject.value("todo").toString();
    task.description = jsonObject.value("description").toString();
    task.project     = parseValue<Project>(jsonObject.value("project"));
    task.activity    = parseValue<Activity>(jsonObject.value("activity"));
//    task.user        = parseValue<User>(jsonObject.value("user"));
    task.endAt       = QDateTime::fromString(jsonObject.value("end").toString(), Qt::ISODate);
    task.estimation  = jsonObject.value("estimation").toInt();

    if (jsonObject.contains("activeTimesheets"))
    {
        const auto& jaTimeSheets = jsonObject.value("activeTimesheets").toArray();
        for (const auto& jvTimeSheet : jaTimeSheets)
        {
            task.activeTimeSheets.push_back(parseValue<TimeSheet>(jvTimeSheet));
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
