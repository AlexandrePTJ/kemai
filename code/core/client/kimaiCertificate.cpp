// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "kimaiCertificate.h"

// Qt headers
#include <QCryptographicHash>

namespace kemai
{
    CertificateInfo CertificateInfo::fromCertificate(const QSslCertificate &certificate, const QString &host)
    {
        CertificateInfo info;
        info.host       = host;
        info.subject    = certificate.subjectDisplayName();
        info.issuer     = certificate.issuerDisplayName();
        info.validFrom  = certificate.effectiveDate();
        info.validUntil = certificate.expiryDate();
        info.sha256     = sha256Of(certificate);
        info.pem        = certificate.toPem();
        return info;
    }

    QString hostKey(const QUrl &url)
    {
        return url.port() > 0 ? QStringLiteral("%1:%2").arg(url.host()).arg(url.port()) : url.host();
    }

    QString sha256Of(const QSslCertificate &certificate)
    {
        return QString::fromLatin1(certificate.digest(QCryptographicHash::Sha256).toHex(':').toUpper());
    }
} // namespace kemai
