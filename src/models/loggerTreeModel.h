#pragma once

#include <QAbstractItemModel>
#include <QDateTime>
#include <QRegularExpression>

#include <spdlog/sinks/base_sink.h>

namespace kemai {

/*
 * Log entry
 */
struct LoggerEntry
{
    QDateTime dateTime;
    QString message;
    spdlog::level::level_enum level;
};

/*
 * Model for Model-View
 */
class LoggerTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    LoggerTreeModel(QObject* parent = nullptr);
    ~LoggerTreeModel() override;

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;

public slots: // NOLINT(readability-redundant-access-specifiers)
    void sinkLog(const kemai::LoggerEntry& entry);

private:
    std::list<LoggerEntry> mEntries;
};

/*
 * Sink to bridge between spdlog and Qt
 */
class LoggerTreeModelSink : public spdlog::sinks::base_sink<std::mutex>
{
public:
    LoggerTreeModelSink(LoggerTreeModel* loggerTreeModel) : mLoggerTreeModel(loggerTreeModel) {}
    ~LoggerTreeModelSink() override = default;

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override
    {
        const auto sysTime    = std::chrono::clock_cast<std::chrono::system_clock>(msg.time);
        const auto timeInMSec = std::chrono::time_point_cast<std::chrono::milliseconds>(sysTime);

        // clang-format off
        LoggerEntry entry{
            QDateTime::fromMSecsSinceEpoch(timeInMSec.time_since_epoch().count(), Qt::UTC),
            QString::fromUtf8(msg.payload.data(), static_cast<int>(msg.payload.size())).remove("<===").remove("===>").trimmed(),
            msg.level};
        // clang-format on

        QMetaObject::invokeMethod(mLoggerTreeModel, "sinkLog", Qt::AutoConnection, Q_ARG(kemai::LoggerEntry, entry));
    }
    void flush_() override {}

private:
    LoggerTreeModel* mLoggerTreeModel;
};

} // namespace kemai
