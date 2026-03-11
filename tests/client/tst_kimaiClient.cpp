// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include <catch2/catch_test_macros.hpp>

#include <QEventLoop>
#include <QFutureWatcher>
#include <QTimer>

#include "client/kimaiClient.h"

using namespace kemai;

template<typename T>
T waitForFuture(QFuture<T> future, int timeoutMs = 10000)
{
    QFutureWatcher<T> watcher;
    QEventLoop        loop;
    QObject::connect(&watcher, &QFutureWatcherBase::finished, &loop, &QEventLoop::quit);
    QTimer::singleShot(timeoutMs, &loop, &QEventLoop::quit);
    watcher.setFuture(future);
    loop.exec();
    return watcher.result();
}

static const QString host  = qEnvironmentVariable("KEMAI_TEST_HOST");
static const QString token = qEnvironmentVariable("KEMAI_TEST_TOKEN");

TEST_CASE("KimaiClient - requestKimaiVersion", "[client][integration]")
{
    if (host.isEmpty() || token.isEmpty())
    {
        SKIP("Set KEMAI_TEST_HOST and KEMAI_TEST_TOKEN to run integration tests");
    }

    KimaiClient client;
    client.setHost(host);
    client.setToken(token);

    auto version = waitForFuture(client.requestKimaiVersion());
    REQUIRE_FALSE(version.kimai.isNull());
}

TEST_CASE("KimaiClient - requestMeUserInfo", "[client][integration]")
{
    if (host.isEmpty() || token.isEmpty())
    {
        SKIP("Set KEMAI_TEST_HOST and KEMAI_TEST_TOKEN to run integration tests");
    }

    KimaiClient client;
    client.setHost(host);
    client.setToken(token);

    auto user = waitForFuture(client.requestMeUserInfo());
    REQUIRE_FALSE(user.username.isEmpty());
}

TEST_CASE("KimaiClient - bad token returns error", "[client][integration]")
{
    if (host.isEmpty())
    {
        SKIP("Set KEMAI_TEST_HOST to run integration tests");
    }

    KimaiClient client;
    client.setHost(host);
    client.setToken("invalid-token");

    REQUIRE_THROWS(waitForFuture(client.requestMeUserInfo()));
}
