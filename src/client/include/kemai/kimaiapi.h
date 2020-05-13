#pragma once

#include <optional>

#include <QDateTime>
#include <QString>
#include <QVector>

namespace kemai::client {

// available requests
enum class ApiMethod
{
    Undefined,
    Version,
    Customers,
    Projects,
    Activities,
    ActiveTimeSheets,
    TimeSheets,
    Users,
    MeUsers
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
    QString kimai = "0.0.0";
};

struct Customer
{
    int id = 0;
    QString name;
    QString countryKey;
    QString currencyKey;
    QString timezone;
};
using Customers = QVector<Customer>;

struct Project
{
    int id;
    QString name;
    Customer customer;
};
using Projects = QVector<Project>;

struct Activity
{
    int id;
    QString name;
    std::optional<Project> project;
};
using Activities = QVector<Activity>;

struct TimeSheet
{
    int id;
    Activity activity;
    Project project;
    QString description;
    QDateTime beginAt;
    QDateTime endAt;
};
using TimeSheets = QVector<TimeSheet>;

struct User
{
    int id;
    QString username;
    QString language;
    QString timezone;
};

QString apiMethodToString(ApiMethod method);

} // namespace kemai::client
