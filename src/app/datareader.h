#pragma once

#include <QMap>
#include <QStringList>

namespace kemai::app {

class DataReader
{
public:
    static QMap<QString, QString> countries();
    static QMap<QString, QString> currencies();
    static QStringList timezones();
};

}
