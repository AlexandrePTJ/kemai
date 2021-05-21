#pragma once

#include <optional>

#include <QDateTime>
#include <QString>
#include <QVector>
#include <QVersionNumber>

namespace kemai::client {

// available requests
enum class ApiMethod
{
    Undefined,
    Version,
    Customers,
    CustomerAdd,
    Projects,
    ProjectAdd,
    Activities,
    ActivityAdd,
    ActiveTimeSheets,
    TimeSheets,
    Users,
    MeUsers,
    Tags,
    Plugins,
    Tasks
};

enum class ApiPlugin
{
    TaskManagement
};

// available verbs
enum class HttpVerb
{
    Get,
    Post,
    Patch
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
using Customers = QVector<Customer>;

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
using Projects = QVector<Project>;

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
using Activities = QVector<Activity>;

struct TimeSheet
{
    int id = 0;
    Activity activity;
    Project project;
    QString description;
    QDateTime beginAt;
    QDateTime endAt;
    Tags tags;
};
using TimeSheets = QVector<TimeSheet>;

struct User
{
    int id = 0;
    QString username;
    QString language;
    QString timezone;
};

struct Task
{
    int id = 0;
    QString title;
    QString status;
    QString todo;
    QString description;
    Project project;
    Activity activity;
    User user;
    QDateTime endAt;
    int estimation = 0;
};
using Tasks = QVector<Task>;

struct Plugin
{
    QString name;
    QVersionNumber version;
};
using Plugins = QVector<Plugin>;

QString apiMethodToString(ApiMethod method);

} // namespace kemai::client
