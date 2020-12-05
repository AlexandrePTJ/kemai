#include "kimaiapi.h"

namespace kemai::client {

QString apiMethodToString(ApiMethod method)
{
    switch (method)
    {
    case ApiMethod::Version:
        return "version";

    case ApiMethod::Customers:
        return "customers";

    case ApiMethod::Projects:
        return "projects";

    case ApiMethod::Activities:
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

    default:
        return "";
    }
}

} // namespace kemai::client
