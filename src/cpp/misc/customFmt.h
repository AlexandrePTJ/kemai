#pragma once

//
#include <fmt/format.h>

// Qt headers
#include <QString>
#include <QUuid>

template<>
struct fmt::formatter<QString>
{
    constexpr auto parse(format_parse_context &ctx)
    { return ctx.end(); }

    template<typename FormatContext>
    auto format(const QString &str, FormatContext &ctx) const
    { return fmt::format_to(ctx.out(), "{}", str.toStdString()); }
};

template<>
struct fmt::formatter<QUuid>
{
    constexpr auto parse(format_parse_context &ctx)
    { return ctx.end(); }

    template<typename FormatContext>
    auto format(const QUuid &str, FormatContext &ctx) const
    { return fmt::format_to(ctx.out(), "{}", str.toString()); }
};
