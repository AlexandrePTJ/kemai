// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "buildInfo.h"

// Qt headers
#include <QSysInfo>
#include <QtVersion>

// Project headers
#include "kemaiConfig.h"

namespace kemai
{
    BuildInfo::BuildInfo(QObject *parent):
    QObject(parent)
    {
    }

    QString BuildInfo::version() const
    {
        return KEMAI_VERSION;
    }

    QString BuildInfo::channel() const
    {
        return KEMAI_CHANNEL;
    }

    QString BuildInfo::qtVersion() const
    {
        // The Qt the app runs with, which may differ from the one it was built with
        return qVersion();
    }

    QString BuildInfo::system() const
    {
        return QString("%1 · %2").arg(QSysInfo::prettyProductName(), QSysInfo::currentCpuArchitecture());
    }

    QUrl BuildInfo::sourceUrl() const
    {
        return QUrl(QString("https://github.com/%1").arg(KEMAI_REPOSITORY));
    }

    QUrl BuildInfo::issuesUrl() const
    {
        return QUrl(QString("https://github.com/%1/issues").arg(KEMAI_REPOSITORY));
    }
} // namespace kemai
