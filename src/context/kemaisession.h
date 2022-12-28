#pragma once

#include "client/kimaiapi.h"

namespace kemai {

struct KemaiSession
{
    QVersionNumber kimaiVersion;
    Plugins plugins;
    User me;
    TimeSheetConfig timeSheetConfig;

    /*
     * Helpers methods
     */
    bool isPluginAvailable(ApiPlugin apiPlugin);
};

} // namespace kemai
