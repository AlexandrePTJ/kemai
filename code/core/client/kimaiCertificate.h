// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// Qt headers
#include <QByteArray>
#include <QDateTime>
#include <QList>
#include <QSslCertificate>
#include <QString>
#include <QUrl>

namespace kemai
{
    // A server certificate the user chose to trust, for one host only (pinned by its SHA-256 fingerprint)
    struct TrustedCertificate
    {
        QString    host;   // as hostKey() gives it
        QString    sha256; // "AB:CD:…"
        QByteArray pem;
        QDateTime  expiresAt;
    };
    using TrustedCertificates = QList<TrustedCertificate>;

    // A server certificate Kemai refused, with what the user needs to decide
    struct CertificateInfo
    {
        enum class Rejection
        {
            None, // only its issuer is unknown: it can be trusted
            Expired,
            NotYetValid,
            HostName, // issued for another name
            Revoked,
            Other
        };

        QString    host;
        QString    subject;
        QString    issuer;
        QDateTime  validFrom;
        QDateTime  validUntil;
        QString    sha256;
        QByteArray pem;
        // Another certificate is trusted for this host: the server's one changed
        bool      replacesTrusted = false;
        Rejection rejection       = Rejection::None;

        static CertificateInfo fromCertificate(const QSslCertificate &certificate, const QString &host);
    };

    // "host" or "host:port" of a server URL: what a trusted certificate is bound to
    QString hostKey(const QUrl &url);
    QString sha256Of(const QSslCertificate &certificate);
} // namespace kemai
