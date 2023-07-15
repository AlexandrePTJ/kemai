#pragma once

#include <QDateTime>
#include <QString>

namespace kemai::helpers {

QString getDurationString(const QDateTime& beginAt, const QDateTime& endAt);

QString getLogDirPath();

QString getLogFilePath();

} // namespace kemai::helpers
