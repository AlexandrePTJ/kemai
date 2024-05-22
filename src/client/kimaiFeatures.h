#pragma once

#include <QVersionNumber>

namespace kemai {

class KimaiFeatures
{
public:
    // Allow current client instance to get instance version and list of available plugins. Only available from Kimai 1.14.1
    static bool canRequestPlugins(const QVersionNumber& kimaiVersion);

    // Starting from 2.14, auth/token headers are deprecated. Should use API Token with Authorization header.
    static bool shouldUseAPIToken(const QVersionNumber& kimaiVersion);
};

} // namespace kemai
