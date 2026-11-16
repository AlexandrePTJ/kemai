// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "logJournal.h"

// Qt headers
#include <QClipboard>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QSysInfo>
#include <QUrl>

// Project headers
#include "kemaiConfig.h"

namespace
{
    QString withHomeShort(const QString &path)
    {
        const auto home = QDir::homePath();
        return path.startsWith(home + '/') ? "~" + path.mid(home.size()) : path;
    }
} // namespace

namespace kemai
{
    LogJournal::LogJournal(QObject *parent):
    QObject(parent),
    m_startedAt(QDateTime::currentDateTime())
    {
        connect(&m_model, &LogEntryModel::countChanged, this, &LogJournal::countChanged);
    }

    LogJournal::~LogJournal() = default;

    LogEntryModel *LogJournal::model()
    {
        return &m_model;
    }

    void LogJournal::setLogFile(const QString &path, const QString &error)
    {
        m_logFilePath = QDir::toNativeSeparators(path);
        m_filesError  = error;
    }

    QAbstractItemModel *LogJournal::entries()
    {
        return &m_model;
    }

    int LogJournal::count() const
    {
        return m_model.rowCount();
    }

    int LogJournal::errorCount() const
    {
        return m_model.errorCount();
    }

    int LogJournal::warningCount() const
    {
        return m_model.warningCount();
    }

    bool LogJournal::filesAvailable() const
    {
        return m_filesError.isEmpty();
    }

    QString LogJournal::filesError() const
    {
        return m_filesError;
    }

    QString LogJournal::logFilePath() const
    {
        return m_logFilePath;
    }

    QString LogJournal::displayFilePath() const
    {
        return withHomeShort(m_logFilePath);
    }

    QString LogJournal::displayFolder() const
    {
        return withHomeShort(QDir::toNativeSeparators(QFileInfo(m_logFilePath).absolutePath()));
    }

    QString LogJournal::reportHeader() const
    {
        return QString("Kemai %1 · %2 · log since %3").arg(KEMAI_VERSION, QSysInfo::prettyProductName(), m_startedAt.toString("yyyy-MM-dd HH:mm:ss"));
    }

    void LogJournal::copyToClipboard(const QString &text) const
    {
        QGuiApplication::clipboard()->setText(text);
    }

    bool LogJournal::openFolder() const
    {
        return QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(m_logFilePath).absolutePath()));
    }
} // namespace kemai
