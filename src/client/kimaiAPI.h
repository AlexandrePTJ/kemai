#pragma once

#include <optional>
#include <vector>

#include <QDateTime>
#include <QString>
#include <QVersionNumber>

namespace kemai {

enum class ApiPlugin
{
    Unknown,
    TaskManagement
};

// reply structs
struct KimaiVersion
{
    QVersionNumber kimai;
};

using Tags = QStringList;

struct Customer
{
    int id       = 0;
    bool visible = true;
    QString name;
    QString number;
    QString comment;
    QString company;
    QString address;
    QString countryKey;
    QString currencyKey;
    QString phone;
    QString fax;
    QString mobile;
    QString email;
    QString homepage;
    QString timezone;
    QString color;
    double budget  = 0.0;
    int timeBudget = 0; // seconds
};
using Customers = std::vector<Customer>;

struct Project
{
    int id       = 0;
    bool visible = true;
    QString name;
    Customer customer;
    QString comment;
    QString orderNumber;
    QString orderDate;
    QString start;
    QString end;
    QString color;
    double budget  = 0.0;
    int timeBudget = 0;
};
using Projects = std::vector<Project>;

struct Activity
{
    int id       = 0;
    bool visible = true;
    QString name;
    QString comment;
    QString color;
    double budget  = 0.0;
    int timeBudget = 0;
    std::optional<Project> project;
};
using Activities = std::vector<Activity>;

struct TimeSheet
{
    int id   = 0;
    int user = 0;
    Activity activity;
    Project project;
    QString description;
    QDateTime beginAt;
    QDateTime endAt;
    Tags tags;
};
using TimeSheets = std::vector<TimeSheet>;

struct User
{
    int id = 0;
    QString username;
    QString language;
    QString timezone;
};

struct Task
{
    enum class Status
    {
        Undefined,
        Pending,
        Progress,
        Closed
    };

    int id = 0;
    QString title;
    Status status = Status::Undefined;
    QString todo;
    QString description;
    Project project;
    Activity activity;
    User user;
    QDateTime endAt;
    int estimation = 0;
    TimeSheets activeTimeSheets;
};
using Tasks = std::vector<Task>;

struct Plugin
{
    QString name;
    QVersionNumber version;
    ApiPlugin apiPlugin;
};
using Plugins = std::vector<Plugin>;

struct TimeSheetConfig
{
    enum class TrackingMode
    {
        Default,
        Punch,
        DurationFixedBegin,
        DurationOnly
    };

    TrackingMode trackingMode = TrackingMode::Default;
};

ApiPlugin pluginByName(const QString& pluginName);

} // namespace kemai
