#include "kimaiFeatures.h"

using namespace kemai;

/*
 * Static helpers
 */
static const auto gKimaiVersionForPluginRequest = QVersionNumber(1, 14, 1);
static const auto gKimaiVersionForAPIToken      = QVersionNumber(2, 14, 0);

/*
 * Class impl
 */
bool KimaiFeatures::canRequestPlugins(const QVersionNumber& kimaiVersion)
{
    return kimaiVersion >= gKimaiVersionForPluginRequest;
}

bool KimaiFeatures::shouldUseAPIToken(const QVersionNumber& kimaiVersion)
{
    return kimaiVersion >= gKimaiVersionForAPIToken;
}
