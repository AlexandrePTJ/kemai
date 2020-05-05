#include "helpers.h"
#include "settings.h"

namespace kemai::app::helpers {

QSharedPointer<client::KimaiClient> createClient()
{
    QSharedPointer<client::KimaiClient> client;

    auto settings = Settings::load();
    if (settings.isReady())
    {
        client = QSharedPointer<client::KimaiClient>::create();
        client->setHost(settings.host);
        client->setUsername(settings.username);
        client->setToken(settings.token);
    }

    return client;
}

} // namespace kemai::app::helpers
