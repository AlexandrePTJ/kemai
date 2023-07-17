#include "helpers.h"

#include <QDir>
#include <QStandardPaths>

namespace kemai::helpers {

QString getDurationString(const QDateTime& beginAt, const QDateTime& endAt)
{
    if (!beginAt.isValid() || !endAt.isValid())
    {
        return "--:--";
    }

    auto nSecs = beginAt.secsTo(endAt);

    // NOLINTBEGIN(readability-magic-numbers)
    const auto nDays = nSecs / 86400;
    nSecs -= nDays * 86400;

    const auto nHours = nSecs / 3600;
    nSecs -= nHours * 3600;

    const auto nMinutes = nSecs / 60;
    nSecs -= nMinutes * 60;

    return QString("%1%2:%3:%4")
        .arg(nDays > 0 ? QString::number(nDays) + "d " : "")
        .arg(nHours, 2, 10, QChar('0'))
        .arg(nMinutes, 2, 10, QChar('0'))
        .arg(nSecs, 2, 10, QChar('0'));
    // NOLINTEND(readability-magic-numbers)
}

QString getLogDirPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

QString getLogFilePath()
{
    return QDir(getLogDirPath()).absoluteFilePath("kemai.log");
}

} // namespace kemai::helpers
