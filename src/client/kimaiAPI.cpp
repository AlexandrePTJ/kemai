#include "kimaiAPI.h"

namespace kemai {

ApiPlugin pluginByName(const QString& pluginName)
{
    if (pluginName == "TaskManagementBundle")
    {
        return ApiPlugin::TaskManagement;
    }
    return ApiPlugin::Unknown;
}

} // namespace kemai
