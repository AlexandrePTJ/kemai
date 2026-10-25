// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include <catch2/catch_test_macros.hpp>

#include <QEventLoop>
#include <QFile>
#include <QFutureWatcher>
#include <QSslKey>
#include <QSslServer>
#include <QSslSocket>
#include <QTimer>

#include "client/kimaiApiError.h"
#include "client/kimaiClient.h"

using namespace kemai;

// A local TLS server answering /api/users/me like Kimai, with the test certificates of data/tls
namespace
{
    QByteArray readTlsFile(const QString &name)
    {
        QFile file(QStringLiteral(KEMAI_TEST_DATA_DIR "/tls/%1").arg(name));
        REQUIRE(file.open(QIODevice::ReadOnly));
        return file.readAll();
    }

    class TlsKimai
    {
    public:
        explicit TlsKimai(const QString &certificateName)
        {
            QSslConfiguration configuration = QSslConfiguration::defaultConfiguration();
            configuration.setLocalCertificate(QSslCertificate(readTlsFile(certificateName + ".pem")));
            configuration.setPrivateKey(QSslKey(readTlsFile(certificateName + ".key"), QSsl::Rsa));
            configuration.setPeerVerifyMode(QSslSocket::VerifyNone);
            m_server.setSslConfiguration(configuration);

            QObject::connect(&m_server,
                             &QSslServer::pendingConnectionAvailable,
                             [this]
                             {
                                 auto *socket = m_server.nextPendingConnection();
                                 QObject::connect(socket,
                                                  &QIODevice::readyRead,
                                                  socket,
                                                  [socket]
                                                  {
                                                      socket->readAll();
                                                      const QByteArray body = R"({"id": 1, "username": "susan", "language": "en", "timezone": "UTC"})";
                                                      socket->write("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\nContent-Length: " + QByteArray::number(body.size()) + "\r\n\r\n" + body);
                                                      socket->disconnectFromHost();
                                                  });
                             });
            REQUIRE(m_server.listen(QHostAddress::LocalHost));
        }

        QString url(const QString &hostName = "127.0.0.1") const
        { return QString("https://%1:%2").arg(hostName).arg(m_server.serverPort()); }
        QString hostKey(const QString &hostName = "127.0.0.1") const
        { return QString("%1:%2").arg(hostName).arg(m_server.serverPort()); }

    private:
        QSslServer m_server;
    };

    TrustedCertificate trusted(const QString &host, const QString &certificateName)
    {
        const QSslCertificate certificate(readTlsFile(certificateName + ".pem"));
        return {.host = host, .sha256 = sha256Of(certificate), .pem = certificate.toPem(), .expiresAt = certificate.expiryDate()};
    }

    // The error a login request ends with, or nullopt when it succeeds
    std::optional<KimaiApiError> loginError(KimaiClient &client)
    {
        auto                      future = client.requestMeUserInfo();
        QFutureWatcher<KimaiUser> watcher;
        QEventLoop                loop;
        QObject::connect(&watcher, &QFutureWatcherBase::finished, &loop, &QEventLoop::quit);
        QTimer::singleShot(10000, &loop, &QEventLoop::quit);
        watcher.setFuture(future);
        loop.exec();
        try
        {
            future.waitForFinished();
            return std::nullopt;
        }
        catch (const KimaiApiError &e)
        {
            return e;
        }
    }
} // namespace

TEST_CASE("Certificate trust - an unknown issuer is refused with the certificate's details", "[certificate][unit]")
{
    TlsKimai server("selfsigned");

    KimaiClient client;
    client.setHost(server.url());
    client.setToken("token");

    const auto error = loginError(client);
    REQUIRE(error.has_value());
    REQUIRE(error->kind() == ApiErrorKind::UntrustedCertificate);
    REQUIRE(error->certificate().has_value());

    const auto &info = *error->certificate();
    REQUIRE(info.host == server.hostKey());
    REQUIRE(info.subject.contains("kemai-test"));
    REQUIRE(info.sha256 == trusted(server.hostKey(), "selfsigned").sha256);
    REQUIRE(info.validUntil.date().year() >= 2120);
    REQUIRE_FALSE(info.replacesTrusted);
}

TEST_CASE("Certificate trust - a certificate trusted for its host is accepted", "[certificate][unit]")
{
    TlsKimai server("selfsigned");

    KimaiClient client;
    client.setHost(server.url());
    client.setToken("token");
    client.setTrustedCertificates({trusted(server.hostKey(), "selfsigned")});

    REQUIRE_FALSE(loginError(client).has_value());
}

TEST_CASE("Certificate trust - trusted for another host is refused", "[certificate][unit]")
{
    TlsKimai server("selfsigned");

    KimaiClient client;
    client.setHost(server.url());
    client.setToken("token");
    client.setTrustedCertificates({trusted("kimai.example.lan", "selfsigned")});

    const auto error = loginError(client);
    REQUIRE(error.has_value());
    REQUIRE(error->kind() == ApiErrorKind::UntrustedCertificate);
    REQUIRE_FALSE(error->certificate()->replacesTrusted);
}

TEST_CASE("Certificate trust - a replaced certificate is refused and said to differ", "[certificate][unit]")
{
    TlsKimai server("other");

    KimaiClient client;
    client.setHost(server.url());
    client.setToken("token");
    client.setTrustedCertificates({trusted(server.hostKey(), "selfsigned")});

    const auto error = loginError(client);
    REQUIRE(error.has_value());
    REQUIRE(error->kind() == ApiErrorKind::UntrustedCertificate);
    REQUIRE(error->certificate()->replacesTrusted);
    REQUIRE(error->certificate()->sha256 == trusted(server.hostKey(), "other").sha256);
}

TEST_CASE("Certificate trust - expired or for another name cannot be trusted", "[certificate][unit]")
{
    SECTION("Expired, even when trusted")
    {
        TlsKimai server("expired");

        KimaiClient client;
        client.setHost(server.url());
        client.setToken("token");
        client.setTrustedCertificates({trusted(server.hostKey(), "expired")});

        const auto error = loginError(client);
        REQUIRE(error.has_value());
        REQUIRE(error->kind() == ApiErrorKind::CertificateRejected);
        REQUIRE(error->certificate()->rejection == CertificateInfo::Rejection::Expired);
    }

    SECTION("Issued for another host name")
    {
        TlsKimai server("wronghost");

        KimaiClient client;
        client.setHost(server.url("localhost"));
        client.setToken("token");
        client.setTrustedCertificates({trusted(server.hostKey("localhost"), "wronghost")});

        const auto error = loginError(client);
        REQUIRE(error.has_value());
        REQUIRE(error->kind() == ApiErrorKind::CertificateRejected);
        REQUIRE(error->certificate()->rejection == CertificateInfo::Rejection::HostName);
    }
}

TEST_CASE("Certificate trust - the certificate's dates count whatever the TLS backend reports", "[certificate][unit]")
{
    const QSslCertificate expired(readTlsFile("expired.pem"));
    const QSslCertificate valid(readTlsFile("selfsigned.pem"));

    // SecureTransport (macOS) reports an expired self-signed certificate as merely untrusted
    const QList<QSslError> untrustedOnly{QSslError(QSslError::CertificateUntrusted, expired)};
    REQUIRE(KimaiClient::certificateRejection(untrustedOnly, expired) == CertificateInfo::Rejection::Expired);

    // OpenSSL lists the expiry itself
    const QList<QSslError> openSsl{QSslError(QSslError::SelfSignedCertificate, expired), QSslError(QSslError::CertificateExpired, expired)};
    REQUIRE(KimaiClient::certificateRejection(openSsl, expired) == CertificateInfo::Rejection::Expired);

    // A valid self-signed certificate only has an unknown issuer: a pin may override it
    REQUIRE(KimaiClient::certificateRejection({QSslError(QSslError::CertificateUntrusted, valid)}, valid) == CertificateInfo::Rejection::None);
    REQUIRE(KimaiClient::certificateRejection({QSslError(QSslError::HostNameMismatch, valid)}, valid) == CertificateInfo::Rejection::HostName);
}
