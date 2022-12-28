#include "kemaisession.h"

using namespace kemai;

bool KemaiSession::isPluginAvailable(ApiPlugin apiPlugin)
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
