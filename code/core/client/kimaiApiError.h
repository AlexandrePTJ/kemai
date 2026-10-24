// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// STL headers
#include <exception>
#include <string>

// Qt headers
#include <QString>

namespace kemai
{
    enum class ApiErrorKind
    {
        NoNetwork,       // offline, timeout, DNS failure, connection refused
        AuthFailed,      // 401 — invalid or expired token
        Forbidden,       // 403 — insufficient permissions
        NotFound,        // 404
        ServerError,     // 5xx
        InvalidResponse, // invalid JSON or unexpected schema
        InvalidValue,    // 400 — Kimai rejected a value; serverMessage holds its message, field() the field when known
        Unknown
    };

    /*!
     * Typed error carried through QFuture failures of IKimaiClient requests.
     * Holds technical data only — user-facing messages are composed at the UI layer.
     */
    class KimaiApiError : public std::exception
    {
    public:
        KimaiApiError(ApiErrorKind kind, int httpStatus, QString endpoint, QString serverMessage, QString field = {});

        ApiErrorKind   kind() const noexcept;
        int            httpStatus() const noexcept;
        const QString &endpoint() const noexcept;
        const QString &serverMessage() const noexcept;
        // Kimai field the message is about (InvalidValue), empty when not tied to a field
        const QString &field() const noexcept;

        const char *what() const noexcept override;

    private:
        ApiErrorKind m_kind;
        int          m_httpStatus;
        QString      m_endpoint;
        QString      m_serverMessage;
        QString      m_field;
        std::string  m_what;
    };
} // namespace kemai
