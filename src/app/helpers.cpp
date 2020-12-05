#include "helpers.h"
#include "settings.h"

namespace kemai::app::helpers {

QSharedPointer<client::KimaiClient> createClient()
{
    QSharedPointer<client::KimaiClient> client;

    auto settings = core::Settings::load();
    if (settings.isReady())
    {
        client = QSharedPointer<client::KimaiClient>::create();
        client->setHost(settings.kimai.host);
        client->setUsername(settings.kimai.username);
        client->setToken(settings.kimai.token);
    }

    return client;
}

} // namespace kemai::app::helpers
