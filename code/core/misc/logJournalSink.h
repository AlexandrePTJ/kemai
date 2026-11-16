// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// STL headers
#include <mutex>

// Qt headers
#include <QPointer>

// 3rd party headers
#include <spdlog/sinks/base_sink.h>

// Project headers
#include <model/logEntryModel.h>

namespace kemai
{
    /*!
     * spdlog sink feeding the log journal. Entries logged from any thread reach the model through its event loop,
     * in order; `detach()` before the model goes away, as the logger may outlive it.
     */
    class LogJournalSink : public spdlog::sinks::base_sink<std::mutex>
    {
    public:
        explicit LogJournalSink(LogEntryModel *model);

        void detach();

        static LogEntryModel::Level levelOf(spdlog::level::level_enum level);

    protected:
        void sink_it_(const spdlog::details::log_msg &msg) override;
        void flush_() override;

    private:
        QPointer<LogEntryModel> m_model;
    };
} // namespace kemai
