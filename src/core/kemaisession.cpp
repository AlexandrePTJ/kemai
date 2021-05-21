#include "kemaisession.h"

using namespace kemai::core;

bool KemaiSession::isPluginAvailable(client::ApiPlugin apiPlugin)
{
    for (const auto& plugin : plugins)
    {
        if (plugin.apiPlugin == apiPlugin)
        {
            return true;
        }
    }
    return false;
}
