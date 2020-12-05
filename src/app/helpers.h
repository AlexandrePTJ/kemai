#pragma once

#include <QSharedPointer>

#include "client/kimaiclient.h"

namespace kemai::app::helpers {

QSharedPointer<client::KimaiClient> createClient();

}
