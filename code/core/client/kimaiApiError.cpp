// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "kimaiApiError.h"

// magic_enum headers
#include <magic_enum/magic_enum.hpp>

// 3rd party headers
#include <fmt/format.h>

namespace kemai
{
    KimaiApiError::KimaiApiError(ApiErrorKind kind, int httpStatus, QString endpoint, QString serverMessage, QString field):
    m_kind(kind),
    m_httpStatus(httpStatus),
    m_endpoint(std::move(endpoint)),
    m_serverMessage(std::move(serverMessage)),
    m_field(std::move(field))
    {
        m_what = fmt::format("[{}] http={} endpoint={} message={}", magic_enum::enum_name(m_kind), m_httpStatus, m_endpoint.toStdString(), m_serverMessage.toStdString());
    }

    ApiErrorKind KimaiApiError::kind() const noexcept
    {
        return m_kind;
    }

    int KimaiApiError::httpStatus() const noexcept
    {
        return m_httpStatus;
    }

    const QString &KimaiApiError::endpoint() const noexcept
    {
        return m_endpoint;
    }

    const QString &KimaiApiError::serverMessage() const noexcept
    {
        return m_serverMessage;
    }

    const QString &KimaiApiError::field() const noexcept
    {
        return m_field;
    }

    const std::optional<CertificateInfo> &KimaiApiError::certificate() const noexcept
    {
        return m_certificate;
    }

    void KimaiApiError::setCertificate(const CertificateInfo &certificate)
    {
        m_certificate = certificate;
    }

    const char *KimaiApiError::what() const noexcept
    {
        return m_what.c_str();
    }
} // namespace kemai
