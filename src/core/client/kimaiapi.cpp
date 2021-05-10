#include "kimaiapi.h"

namespace kemai::client {

QString apiMethodToString(ApiMethod method)
{
    switch (method)
    {
    case ApiMethod::Version:
        return "version";

    case ApiMethod::Customers:
    case ApiMethod::CustomerAdd:
        return "customers";

    case ApiMethod::Projects:
    case ApiMethod::ProjectAdd:
        return "projects";

    case ApiMethod::Activities:
    case ApiMethod::ActivityAdd:
        return "activities";

    case ApiMethod::ActiveTimeSheets:
        return "timesheets/active";

    case ApiMethod::TimeSheets:
        return "timesheets";

    case ApiMethod::Users:
        return "users";

    case ApiMethod::MeUsers:
        return "users/me";

    case ApiMethod::Tags:
        return "tags";

    case ApiMethod::Tasks:
        return "tasks";

    case ApiMethod::Plugins:
        return "plugins";

    default:
        return "";
    }
}

} // namespace kemai::client
