#include "kimaiAPI.h"

namespace kemai {

ApiPlugin pluginByName(const QString& pluginName)
{
    if (pluginName == "TaskManagementBundle")
    {
        return ApiPlugin::TaskManagement;
    }
    if (pluginName == "MetaFieldsBundle")
    {
        return ApiPlugin::MetaFields;
    }
    return ApiPlugin::Unknown;
}

} // namespace kemai
