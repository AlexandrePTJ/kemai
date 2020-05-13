#pragma once

#include <QMap>

namespace kemai::app {

class DataReader
{
public:
    static QMap<QString, QString> countries();
    static QMap<QString, QString> currencies();
};

}
