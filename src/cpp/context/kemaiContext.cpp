// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "kemaiContext.h"

// Project headers
#include "client/kimaiSystem.h"

namespace kemai
{
    KemaiContext::KemaiContext(QObject *parent):
    QObject(parent)
    {
    }

    KemaiContext::~KemaiContext() = default;

    void KemaiContext::initialize()
    {
    }

    void KemaiContext::login(const QString &host, const QString &token)
    {
        m_pendingClient = std::make_unique<KimaiClient>();
        m_pendingClient->setHost(host);
        m_pendingClient->setToken(token);

        m_pendingClient->requestMeUserInfo()
            .then(this, [this, host](KimaiUser user)
                  {
                auto* session = new SessionContext(std::move(m_pendingClient), host, user, this);
                emit loginSucceeded(session); })
            .onFailed(this, [this](const std::exception &e)
                      {
                m_pendingClient.reset();
                emit loginFailed(QString::fromStdString(e.what())); });
    }

} // namespace kemai
