#pragma once

#include <QSharedPointer>

#include <kemai/kimaiclient.h>

namespace kemai::app::helpers {

QSharedPointer<client::KimaiClient> createClient();

}
