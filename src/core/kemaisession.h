#pragma once

#include "client/kimaiapi.h"

namespace kemai::core {

struct KemaiSession
{
    QVersionNumber kimaiVersion;
    client::Plugins plugins;
    client::User me;
    client::TimeSheetConfig timeSheetConfig;

    /*
     * Helpers methods
     */
    bool isPluginAvailable(client::ApiPlugin apiPlugin);
};

} // namespace kemai::core
