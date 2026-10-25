// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include <catch2/catch_test_macros.hpp>

#include <QEventLoop>
#include <QFile>
#include <QFutureWatcher>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>

#include "client/kemaiUpdateChecker.h"

using namespace kemai;
using Channel = KemaiUpdateChecker::Channel;

namespace
{
    QJsonDocument recorded(const QString &name)
    {
        QFile file(QStringLiteral(KEMAI_TEST_DATA_DIR "/github-releases/%1").arg(name));
        REQUIRE(file.open(QIODevice::ReadOnly));
        return QJsonDocument::fromJson(file.readAll());
    }

    // A local HTTP server answering every request with `status` and `body`
    class FakeGitHub
    {
    public:
        FakeGitHub(int status, QByteArray body)
        {
            QObject::connect(&m_server,
                             &QTcpServer::newConnection,
                             [this, status, body]
                             {
                                 auto *socket = m_server.nextPendingConnection();
                                 QObject::connect(socket,
                                                  &QIODevice::readyRead,
                                                  socket,
                                                  [socket, status, body]
                                                  {
                                                      socket->readAll();
                                                      socket->write("HTTP/1.1 " + QByteArray::number(status) + " X\r\nContent-Type: application/json\r\nConnection: close\r\nContent-Length: " + QByteArray::number(body.size()) + "\r\n\r\n" + body);
                                                      socket->disconnectFromHost();
                                                  });
                             });
            REQUIRE(m_server.listen(QHostAddress::LocalHost));
        }

        QUrl url() const
        { return QUrl(QString("http://127.0.0.1:%1/releases").arg(m_server.serverPort())); }

    private:
        QTcpServer m_server;
    };

    template<typename T>
    void wait(const QFuture<T> &future)
    {
        QFutureWatcher<T> watcher;
        QEventLoop        loop;
        QObject::connect(&watcher, &QFutureWatcherBase::finished, &loop, &QEventLoop::quit);
        QTimer::singleShot(10000, &loop, &QEventLoop::quit);
        watcher.setFuture(future);
        loop.exec();
    }
} // namespace

TEST_CASE("KemaiUpdateChecker - calendar versions only", "[updates][unit]")
{
    REQUIRE(KemaiUpdateChecker::isCalendarVersion("2026.11.1"));
    REQUIRE(KemaiUpdateChecker::isCalendarVersion("2026.12.10"));
    REQUIRE_FALSE(KemaiUpdateChecker::isCalendarVersion("0.11.1"));
    REQUIRE_FALSE(KemaiUpdateChecker::isCalendarVersion("2026.13.1"));
    REQUIRE_FALSE(KemaiUpdateChecker::isCalendarVersion("v2026.11.1"));
    REQUIRE_FALSE(KemaiUpdateChecker::isCalendarVersion("98-break-ui"));
}

TEST_CASE("KemaiUpdateChecker::newestRelease - channels, old tags, drafts", "[updates][unit]")
{
    const auto releases = recorded("with-calendar-releases.json");

    SECTION("An alpha sees the newer pre-release, never the draft")
    {
        const auto newest = KemaiUpdateChecker::newestRelease(releases, "2026.11.1", Channel::Alpha);
        REQUIRE(newest.has_value());
        REQUIRE(newest->version == "2026.12.1");
        REQUIRE(newest->prerelease);
        REQUIRE(newest->url == QUrl("https://github.com/AlexandrePTJ/kemai/releases/tag/2026.12.1"));
    }

    SECTION("A stable build ignores pre-releases")
    {
        const auto newest = KemaiUpdateChecker::newestRelease(releases, "2026.11.1", Channel::Stable);
        REQUIRE(newest.has_value());
        REQUIRE(newest->version == "2026.11.2");
        REQUIRE_FALSE(KemaiUpdateChecker::newestRelease(releases, "2026.11.2", Channel::Stable).has_value());
    }

    SECTION("Up to date")
    {
        REQUIRE_FALSE(KemaiUpdateChecker::newestRelease(releases, "2026.12.1", Channel::Alpha).has_value());
    }

    SECTION("0.11.x and other tags are never updates")
    {
        const auto onlyOld = recorded("kemai-2026-10-25.json");
        REQUIRE_FALSE(KemaiUpdateChecker::newestRelease(onlyOld, "2026.11.1", Channel::Alpha).has_value());
        REQUIRE_FALSE(KemaiUpdateChecker::newestRelease(onlyOld, "2026.11.1", Channel::Stable).has_value());
    }

    SECTION("A development build is older than any release")
    {
        const auto newest = KemaiUpdateChecker::newestRelease(releases, "dev-1a2b3c4", Channel::Dev);
        REQUIRE(newest.has_value());
        REQUIRE(newest->version == "2026.12.1");
    }

    SECTION("An ignored version is skipped, a later one reported")
    {
        REQUIRE_FALSE(KemaiUpdateChecker::newestRelease(releases, "2026.11.1", Channel::Alpha, "2026.12.1").has_value());
        // Ignoring a newer one silences the older ones too
        REQUIRE_FALSE(KemaiUpdateChecker::newestRelease(releases, "2026.11.1", Channel::Stable, "2026.12.1").has_value());
        const auto later = KemaiUpdateChecker::newestRelease(releases, "2026.11.1", Channel::Alpha, "2026.11.2");
        REQUIRE(later.has_value());
        REQUIRE(later->version == "2026.12.1");
    }

    SECTION("Malformed replies fail")
    {
        REQUIRE_THROWS_AS(KemaiUpdateChecker::newestRelease(QJsonDocument::fromJson(R"({"message": "Not Found"})"), "2026.11.1", Channel::Alpha), UpdateCheckError);
        REQUIRE_THROWS_AS(KemaiUpdateChecker::newestRelease(QJsonDocument::fromJson(R"([{"name": "x"}])"), "2026.11.1", Channel::Alpha), UpdateCheckError);
        REQUIRE_THROWS_AS(KemaiUpdateChecker::newestRelease(QJsonDocument(), "2026.11.1", Channel::Alpha), UpdateCheckError);
    }
}

TEST_CASE("KemaiUpdateChecker::check - over HTTP", "[updates][unit]")
{
    KemaiUpdateChecker checker;

    SECTION("Newer release found")
    {
        QFile file(QStringLiteral(KEMAI_TEST_DATA_DIR "/github-releases/with-calendar-releases.json"));
        REQUIRE(file.open(QIODevice::ReadOnly));
        FakeGitHub github(200, file.readAll());
        checker.setReleasesUrl(github.url());

        auto future = checker.check("2026.11.1", Channel::Alpha);
        wait(future);
        REQUIRE(future.result().has_value());
        REQUIRE(future.result()->version == "2026.12.1");
    }

    SECTION("Rate limited")
    {
        FakeGitHub github(403, R"({"message": "API rate limit exceeded"})");
        checker.setReleasesUrl(github.url());

        auto future = checker.check("2026.11.1", Channel::Alpha);
        wait(future);
        try
        {
            future.waitForFinished();
            FAIL("check should fail");
        }
        catch (const UpdateCheckError &e)
        {
            REQUIRE(e.kind() == UpdateCheckError::Kind::ServerError);
        }
    }

    SECTION("Nobody answers")
    {
        quint16 port = 0;
        {
            QTcpServer closed;
            REQUIRE(closed.listen(QHostAddress::LocalHost));
            port = closed.serverPort();
        }
        checker.setReleasesUrl(QUrl(QString("http://127.0.0.1:%1/releases").arg(port)));

        auto future = checker.check("2026.11.1", Channel::Alpha);
        wait(future);
        try
        {
            future.waitForFinished();
            FAIL("check should fail");
        }
        catch (const UpdateCheckError &e)
        {
            REQUIRE(e.kind() == UpdateCheckError::Kind::NoNetwork);
        }
    }
}
