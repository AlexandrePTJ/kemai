#include "kimairequestfactory.h"

#include "parser.h"

using namespace kemai::client;

KimaiRequest KimaiRequestFactory::version()
{
    return KimaiRequest(ApiMethod::Version);
}

KimaiRequest KimaiRequestFactory::me()
{
    return KimaiRequest(ApiMethod::MeUsers);
}

KimaiRequest KimaiRequestFactory::timeSheetConfig()
{
    return KimaiRequest(ApiMethod::TimeSheetConfig);
}

KimaiRequest KimaiRequestFactory::customers()
{
    return KimaiRequest(ApiMethod::Customers);
}

KimaiRequest KimaiRequestFactory::customerAdd(const Customer& customer)
{
    auto krq = KimaiRequest(ApiMethod::CustomerAdd, HttpVerb::Post);

    auto jsData = parser::toJson(customer);
    krq.setData(parser::toPostData(jsData));

    return krq;
}

KimaiRequest KimaiRequestFactory::projects()
{
    return KimaiRequest(ApiMethod::Projects);
}

KimaiRequest KimaiRequestFactory::projects(int customerId)
{
    auto krq = KimaiRequest(ApiMethod::Projects);
    krq.setParameters({{"customer", QString::number(customerId)}});
    return krq;
}

KimaiRequest KimaiRequestFactory::projectAdd(const Project& project)
{
    auto krq = KimaiRequest(ApiMethod::ProjectAdd, HttpVerb::Post);

    auto jsData = parser::toJson(project);
    krq.setData(parser::toPostData(jsData));

    return krq;
}

KimaiRequest KimaiRequestFactory::activities()
{
    return KimaiRequest(ApiMethod::Activities);
}

KimaiRequest KimaiRequestFactory::activities(int projectId)
{
    auto krq = KimaiRequest(ApiMethod::Activities);
    krq.setParameters({{"project", QString::number(projectId)}});
    return krq;
}

KimaiRequest KimaiRequestFactory::activityAdd(const Activity& activity)
{
    auto krq = KimaiRequest(ApiMethod::ActivityAdd, HttpVerb::Post);

    auto jsData = parser::toJson(activity);
    krq.setData(parser::toPostData(jsData));

    return krq;
}

KimaiRequest KimaiRequestFactory::activeTimeSheets()
{
    return KimaiRequest(ApiMethod::ActiveTimeSheets);
}

KimaiRequest KimaiRequestFactory::startTimeSheet(int projectId, int activityId, const QDateTime& beginAt, const QString& description, const QStringList& tags,
                                                 TimeSheetConfig::TrackingMode trackingMode)
{
    auto krq = KimaiRequest(ApiMethod::TimeSheets, HttpVerb::Post);

    TimeSheet ts;
    ts.beginAt     = beginAt;
    ts.activity.id = activityId;
    ts.project.id  = projectId;
    ts.tags        = tags;
    ts.description = description;

    auto jsData = parser::toJson(ts, trackingMode);
    krq.setData(parser::toPostData(jsData));

    return krq;
}

KimaiRequest KimaiRequestFactory::stopTimeSheet(int timeSheetId)
{
    auto krq = KimaiRequest(ApiMethod::TimeSheets, HttpVerb::Patch);
    krq.setPatchVerb(QString("%1/stop").arg(timeSheetId));
    return krq;
}

KimaiRequest KimaiRequestFactory::tags()
{
    return KimaiRequest(ApiMethod::Tags);
}

KimaiRequest KimaiRequestFactory::tasks()
{
    return KimaiRequest(ApiMethod::Tasks);
}

KimaiRequest KimaiRequestFactory::taskStart(int taskId)
{
    auto krq = KimaiRequest(ApiMethod::TaskStart, HttpVerb::Patch);
    krq.setPatchVerb(QString("%1/start").arg(taskId));
    return krq;
}

KimaiRequest KimaiRequestFactory::taskStop(int taskId)
{
    auto krq = KimaiRequest(ApiMethod::TaskStop, HttpVerb::Patch);
    krq.setPatchVerb(QString("%1/stop").arg(taskId));
    return krq;
}

KimaiRequest KimaiRequestFactory::taskClose(int taskId)
{
    auto krq = KimaiRequest(ApiMethod::TaskClose, HttpVerb::Patch);
    krq.setPatchVerb(QString("%1/close").arg(taskId));
    return krq;
}

KimaiRequest KimaiRequestFactory::plugins()
{
    return KimaiRequest(ApiMethod::Plugins);
}
