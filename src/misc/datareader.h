#pragma once

#include <QMap>

namespace kemai {

class DataReader
{
public:
    static QMap<QString, QString> countries();
    static QMap<QString, QString> currencies();
};

}
