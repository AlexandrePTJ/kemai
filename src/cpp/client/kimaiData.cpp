// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "kimaiData.h"

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
