// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "logJournalSink.h"

// STL headers
#include <chrono>

// Qt headers
#include <QMetaObject>

namespace kemai
{
    LogJournalSink::LogJournalSink(LogEntryModel *model):
    m_model(model)
    {
    }

    void LogJournalSink::detach()
    {
        std::lock_guard lock(mutex_);
        m_model.clear();
    }

    LogEntryModel::Level LogJournalSink::levelOf(spdlog::level::level_enum level)
    {
        switch (level)
        {
        case spdlog::level::trace:
        case spdlog::level::debug:
            return LogEntryModel::Level::Debug;
        case spdlog::level::info:
            return LogEntryModel::Level::Info;
        case spdlog::level::warn:
            return LogEntryModel::Level::Warning;
        default:
            return LogEntryModel::Level::Error;
        }
    }

    void LogJournalSink::sink_it_(const spdlog::details::log_msg &msg)
    {
        if (m_model.isNull())
        {
            return;
        }

        const auto           milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(msg.time.time_since_epoch()).count();
        LogEntryModel::Entry entry{.time    = QDateTime::fromMSecsSinceEpoch(milliseconds),
                                   .level   = levelOf(msg.level),
                                   .message = QString::fromUtf8(msg.payload.data(), static_cast<qsizetype>(msg.payload.size()))};

        // Queued even on the model's thread: a log line written while the journal updates must not re-enter it
        QMetaObject::invokeMethod(
            m_model.data(),
            [model = m_model, entry = std::move(entry)]
            {
                if (!model.isNull())
                {
                    model->append(entry);
                }
            },
            Qt::QueuedConnection);
    }

    void LogJournalSink::flush_()
    {
    }
} // namespace kemai
