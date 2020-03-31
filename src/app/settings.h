#pragma once

#include <QString>

namespace kemai::app {

struct Settings
{
    QString host;
    QString username;
    QString token;
    bool closeToSystemTray;

    bool isReady() const;

    static Settings load(const QString& confPath = "");
    static void save(const Settings& settings, const QString& confPath = "");
};

} // namespace kemai::app
