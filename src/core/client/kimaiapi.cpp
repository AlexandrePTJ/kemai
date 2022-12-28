#include "kimaiapi.h"

namespace kemai::client {

ApiPlugin pluginByName(const QString& pluginName)
{
    if (pluginName == "TaskManagementBundle")
    {
        return ApiPlugin::TaskManagement;
    }
    return ApiPlugin::Unknown;
}

} // namespace kemai::client
