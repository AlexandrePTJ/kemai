#include "kimairequestfactory.h"

#include "parser.h"

using namespace kemai::client;

KimaiRequest KimaiRequestFactory::version()
{
    return std::move(KimaiRequest(ApiMethod::Version));
}

KimaiRequest KimaiRequestFactory::me()
{
    return std::move(KimaiRequest(ApiMethod::MeUsers));
}

KimaiRequest KimaiRequestFactory::customers()
{
    return std::move(KimaiRequest(ApiMethod::Customers));
}

KimaiRequest KimaiRequestFactory::customerAdd(const Customer& customer)
{
    auto krq = KimaiRequest(ApiMethod::Customers, HttpVerb::Post);

    auto jsData = parser::toJson(customer);
    krq.setData(parser::toPostData(jsData));

    return std::move(krq);
}

KimaiRequest KimaiRequestFactory::projects(int customerId)
{
    auto krq = KimaiRequest(ApiMethod::Projects);
    krq.setParameters({{"customer", QString::number(customerId)}});
    return std::move(krq);
}

KimaiRequest KimaiRequestFactory::projectAdd(const Project& project)
{
    auto krq = KimaiRequest(ApiMethod::Projects, HttpVerb::Post);

    auto jsData = parser::toJson(project);
    krq.setData(parser::toPostData(jsData));

    return std::move(krq);
}

KimaiRequest KimaiRequestFactory::activities(int projectId)
{
    auto krq = KimaiRequest(ApiMethod::Activities);
    krq.setParameters({{"project", QString::number(projectId)}});
    return std::move(krq);
}

KimaiRequest KimaiRequestFactory::activityAdd(const Activity& activity)
{
    auto krq = KimaiRequest(ApiMethod::Activities, HttpVerb::Post);

    auto jsData = parser::toJson(activity);
    krq.setData(parser::toPostData(jsData));

    return std::move(krq);
}

KimaiRequest KimaiRequestFactory::activeTimeSheets()
{
    return std::move(KimaiRequest(ApiMethod::ActiveTimeSheets));
}

KimaiRequest KimaiRequestFactory::startTimeSheet(int projectId, int activityId, const QDateTime& beginAt,
                                                 const QString& description, const QStringList& tags)
{
    auto krq = KimaiRequest(ApiMethod::TimeSheets, HttpVerb::Post);

    TimeSheet ts;
    ts.beginAt     = beginAt;
    ts.activity.id = activityId;
    ts.project.id  = projectId;
    ts.tags        = tags;
    ts.description = description;

    auto jsData = parser::toJson(ts);
    krq.setData(parser::toPostData(jsData));

    return std::move(krq);
}

KimaiRequest KimaiRequestFactory::stopTimeSheet(int timeSheetId)
{
    auto krq = KimaiRequest(ApiMethod::TimeSheets, HttpVerb::Patch);
    krq.setPatchVerb(QString("%1/stop").arg(timeSheetId));
    return std::move(krq);
}

KimaiRequest KimaiRequestFactory::tags()
{
    return std::move(KimaiRequest(ApiMethod::Tags));
}
