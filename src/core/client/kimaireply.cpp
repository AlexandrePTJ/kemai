#include "kimaireply.h"

#include "parser.h"

#include <QJsonArray>
#include <QJsonDocument>

using namespace kemai::client;

class KimaiReply::KimaiReplyPrivate
{
public:
    KimaiReplyPrivate() = default;

    KimaiReplyPrivate(ApiMethod method, const QByteArray& data) : apiMethod(method)
    {
        QJsonParseError parseError;
        auto jdoc = QJsonDocument::fromJson(data, &parseError);
        if (parseError.error == QJsonParseError::NoError)
        {
            if (jdoc.isArray())
            {
                jsonValue = jdoc.array();
            }
            else if (jdoc.isObject())
            {
                jsonValue = jdoc.object();
            }
        }
    }

    bool isValid() const { return apiMethod != ApiMethod::Undefined && !jsonValue.isNull(); }

    ApiMethod apiMethod;
    QJsonValue jsonValue;
};

KimaiReply::KimaiReply(ApiMethod method, const QByteArray& data) : mD(new KimaiReplyPrivate(method, data)) {}

KimaiReply::~KimaiReply() = default;

KimaiReply::KimaiReply(const KimaiReply& rhs) : mD(new KimaiReplyPrivate)
{
    mD->apiMethod = rhs.mD->apiMethod;
    mD->jsonValue = rhs.mD->jsonValue;
}

KimaiReply::KimaiReply(KimaiReply&& rhs) noexcept : mD(new KimaiReplyPrivate)
{
    mD->apiMethod = rhs.mD->apiMethod;
    mD->jsonValue = std::move(rhs.mD->jsonValue);
}

KimaiReply& KimaiReply::operator=(const KimaiReply& rhs)
{
    mD->apiMethod = rhs.mD->apiMethod;
    mD->jsonValue = rhs.mD->jsonValue;
    return *this;
}

KimaiReply& KimaiReply::operator=(KimaiReply&& rhs) noexcept
{
    mD->apiMethod = rhs.mD->apiMethod;
    mD->jsonValue = std::move(rhs.mD->jsonValue);
    return *this;
}

ApiMethod KimaiReply::method() const
{
    return mD->apiMethod;
}

bool KimaiReply::isValid() const
{
    return mD->isValid();
}

// parsers
template<> KimaiVersion KimaiReply::get() const
{
    KimaiVersion kv;
    parser::fromJson(mD->jsonValue.toObject(), kv);
    return kv;
}

template<> Customer KimaiReply::get() const
{
    Customer ts;
    parser::fromJson(mD->jsonValue.toObject(), ts);
    return ts;
}

template<> Customers KimaiReply::get() const
{
    Customers customers;
    for (const auto& jscusto : mD->jsonValue.toArray())
    {
        Customer customer;
        if (parser::fromJson(jscusto.toObject(), customer))
        {
            customers << customer;
        }
    }
    return customers;
}

template<> Project KimaiReply::get() const
{
    Project ts;
    parser::fromJson(mD->jsonValue.toObject(), ts);
    return ts;
}

template<> Projects KimaiReply::get() const
{
    Projects projects;
    for (const auto& jsproject : mD->jsonValue.toArray())
    {
        Project project;
        if (parser::fromJson(jsproject.toObject(), project))
        {
            projects << project;
        }
    }
    return projects;
}

template<> Activity KimaiReply::get() const
{
    Activity ts;
    parser::fromJson(mD->jsonValue.toObject(), ts);
    return ts;
}

template<> Activities KimaiReply::get() const
{
    Activities activities;
    for (const auto& jsactivity : mD->jsonValue.toArray())
    {
        Activity activity;
        if (parser::fromJson(jsactivity.toObject(), activity))
        {
            activities << activity;
        }
    }
    return activities;
}

template<> TimeSheet KimaiReply::get() const
{
    TimeSheet ts;
    parser::fromJson(mD->jsonValue.toObject(), ts);
    return ts;
}

template<> TimeSheets KimaiReply::get() const
{
    TimeSheets timeSheets;
    for (const auto& jstimeSheet : mD->jsonValue.toArray())
    {
        TimeSheet timeSheet;
        if (parser::fromJson(jstimeSheet.toObject(), timeSheet))
        {
            timeSheets << timeSheet;
        }
    }
    return timeSheets;
}

template<> User KimaiReply::get() const
{
    User user;
    parser::fromJson(mD->jsonValue.toObject(), user);
    return user;
}

template<> Tags KimaiReply::get() const
{
    Tags tags;
    for (const auto& jstag : mD->jsonValue.toArray())
    {
        tags << jstag.toString();
    }
    return tags;
}

template<> Tasks KimaiReply::get() const
{
    Tasks tasks;
    for (const auto& jsTask : mD->jsonValue.toArray())
    {
        Task task;
        if (parser::fromJson(jsTask.toObject(), task))
        {
            tasks << task;
        }
    }
    return tasks;
}

template<> Plugins KimaiReply::get() const
{
    Plugins plugins;
    for (const auto& jsPlugin : mD->jsonValue.toArray())
    {
        Plugin plugin;
        if (parser::fromJson(jsPlugin.toObject(), plugin))
        {
            plugins << plugin;
        }
    }
    return plugins;
}

template<> TimeSheetConfig KimaiReply::get() const
{
    TimeSheetConfig tsConfig;
    parser::fromJson(mD->jsonValue.toObject(), tsConfig);
    return tsConfig;
}
