// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// Qt headers
#include <QAbstractItemModel>
#include <QDateTime>
#include <QObject>
#include <QString>
#include <QtQmlIntegration/qqmlintegration.h>

// Project headers
#include <model/logEntryModel.h>

namespace kemai
{
    /*!
     * The log of the current run for the log journal: its entries (fed by LogJournalSink) and where the log files
     * are. Created by main() before the logger and handed to Main.qml as its `logJournal` property.
     */
    class LogJournal : public QObject
    {
        Q_OBJECT
        QML_ELEMENT
        QML_UNCREATABLE("Created by main(), passed to Main.qml")

        Q_PROPERTY(QAbstractItemModel *entries READ entries CONSTANT)
        Q_PROPERTY(int count READ count NOTIFY countChanged)
        Q_PROPERTY(int errorCount READ errorCount NOTIFY countChanged)
        Q_PROPERTY(int warningCount READ warningCount NOTIFY countChanged)
        // Current log file; false with `filesError` (the reason) when it cannot be written
        Q_PROPERTY(bool filesAvailable READ filesAvailable CONSTANT)
        Q_PROPERTY(QString filesError READ filesError CONSTANT)
        Q_PROPERTY(QString logFilePath READ logFilePath CONSTANT)
        // Same paths with the home folder shown as "~"
        Q_PROPERTY(QString displayFilePath READ displayFilePath CONSTANT)
        Q_PROPERTY(QString displayFolder READ displayFolder CONSTANT)

    public:
        explicit LogJournal(QObject *parent = nullptr);
        ~LogJournal() override;

        LogEntryModel *model();
        void           setLogFile(const QString &path, const QString &error);

        QAbstractItemModel *entries();
        int                 count() const;
        int                 errorCount() const;
        int                 warningCount() const;
        bool                filesAvailable() const;
        QString             filesError() const;
        QString             logFilePath() const;
        QString             displayFilePath() const;
        QString             displayFolder() const;

        // First line of a copied log: "Kemai <version> · <OS> · log since <start>"
        Q_INVOKABLE QString reportHeader() const;
        Q_INVOKABLE void    copyToClipboard(const QString &text) const;
        // False when the system has nothing to open it with
        Q_INVOKABLE bool openFolder() const;

    signals:
        void countChanged();

    private:
        LogEntryModel m_model;
        QDateTime     m_startedAt;
        QString       m_logFilePath;
        QString       m_filesError;
    };
} // namespace kemai
