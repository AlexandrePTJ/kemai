// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// Qt headers
#include <QObject>
#include <QString>
#include <QUrl>
#include <QtQmlIntegration/qqmlintegration.h>

namespace kemai
{
    // Which Kemai runs, for the About dialog: build version and channel, Qt and system at run time, project links
    class BuildInfo : public QObject
    {
        Q_OBJECT
        QML_ELEMENT

        // Release version, or dev-<commit> for a development build
        Q_PROPERTY(QString version READ version CONSTANT)
        // "alpha", "stable" or "dev"
        Q_PROPERTY(QString channel READ channel CONSTANT)
        Q_PROPERTY(QString qtVersion READ qtVersion CONSTANT)
        // e.g. "Arch Linux · x86_64"
        Q_PROPERTY(QString system READ system CONSTANT)
        Q_PROPERTY(QUrl sourceUrl READ sourceUrl CONSTANT)
        Q_PROPERTY(QUrl issuesUrl READ issuesUrl CONSTANT)

    public:
        explicit BuildInfo(QObject *parent = nullptr);

        QString version() const;
        QString channel() const;
        QString qtVersion() const;
        QString system() const;
        QUrl    sourceUrl() const;
        QUrl    issuesUrl() const;
    };
} // namespace kemai
