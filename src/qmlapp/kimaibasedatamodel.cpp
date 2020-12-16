#include "kimaibasedatamodel.h"

#include "client/kimaiapi.h"

using namespace kemai::client;
using namespace kemai::qmlapp;

template<> std::optional<int> KimaiBaseDataModel<Customer>::customerId(const Customer& t) const
{
    return t.id;
}

template<> std::optional<int> KimaiBaseDataModel<Project>::customerId(const Project& t) const
{
    return t.customer.id;
}

template<> std::optional<int> KimaiBaseDataModel<Activity>::customerId(const Activity& t) const
{
    if (t.project)
        return t.project->customer.id;
    return std::nullopt;
}

template<> std::optional<int> KimaiBaseDataModel<Customer>::projectId(const Customer& t) const
{
    return std::nullopt;
}

template<> std::optional<int> KimaiBaseDataModel<Project>::projectId(const Project& t) const
{
    return t.id;
}

template<> std::optional<int> KimaiBaseDataModel<Activity>::projectId(const Activity& t) const
{
    if (t.project)
        return t.project->id;
    return std::nullopt;
}

template<> std::optional<int> KimaiBaseDataModel<Customer>::activityId(const Customer& t) const
{
    return std::nullopt;
}

template<> std::optional<int> KimaiBaseDataModel<Project>::activityId(const Project& t) const
{
    return std::nullopt;
}

template<> std::optional<int> KimaiBaseDataModel<Activity>::activityId(const Activity& t) const
{
    return t.id;
}
