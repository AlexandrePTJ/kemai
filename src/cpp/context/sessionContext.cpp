// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "sessionContext.h"

namespace kemai
{
    SessionContext::SessionContext(std::unique_ptr<KimaiClient> client, const QString &host, const KimaiUser &user, QObject *parent):
    QObject(parent),
    m_client(std::move(client)),
    m_host(host),
    m_user(user)
    {
    }

    SessionContext::~SessionContext() = default;

    QString SessionContext::host() const
    {
        return m_host;
    }

    QString SessionContext::username() const
    {
        return m_user.username;
    }

} // namespace kemai
