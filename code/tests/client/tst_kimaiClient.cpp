// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include <catch2/catch_test_macros.hpp>
#include <spdlog/sinks/ringbuffer_sink.h>
#include <spdlog/spdlog.h>

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFutureWatcher>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QUrlQuery>

#include "client/kimaiApiError.h"
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

// Body-less futures: waitForFinished() rethrows the stored KimaiApiError, if any
void waitForFuture(QFuture<void> future, int timeoutMs = 10000)
{
    QFutureWatcher<void> watcher;
    QEventLoop           loop;
    QObject::connect(&watcher, &QFutureWatcherBase::finished, &loop, &QEventLoop::quit);
    QTimer::singleShot(timeoutMs, &loop, &QEventLoop::quit);
    watcher.setFuture(future);
    loop.exec();
    future.waitForFinished();
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

TEST_CASE("KimaiClient - silent server times out as NoNetwork", "[client][unit]")
{
    // Accepts connections and never answers, like a connection whose network went away
    QTcpServer server;
    REQUIRE(server.listen(QHostAddress::LocalHost));

    KimaiClient client;
    client.setHost(QString("http://127.0.0.1:%1").arg(server.serverPort()));
    client.setToken("token");
    client.setTransferTimeout(std::chrono::milliseconds(300));

    auto kind = ApiErrorKind::Unknown;
    try
    {
        waitForFuture(client.requestKimaiVersion(), 5000);
    }
    catch (const KimaiApiError &e)
    {
        kind = e.kind();
    }
    REQUIRE(kind == ApiErrorKind::NoNetwork);
}

TEST_CASE("KimaiClient - token and URL credentials never reach the log", "[client][unit]")
{
    QTcpServer server;
    REQUIRE(server.listen(QHostAddress::LocalHost));

    // Capture everything logged while the request runs
    auto capture  = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(1000);
    auto previous = spdlog::default_logger();
    spdlog::set_default_logger(std::make_shared<spdlog::logger>("capture", capture));
    spdlog::set_level(spdlog::level::trace);

    {
        KimaiClient client;
        client.setHost(QString("http://someone:url-password@127.0.0.1:%1").arg(server.serverPort()));
        client.setToken("secret-api-token-1234");
        client.setTransferTimeout(std::chrono::milliseconds(300));

        try
        {
            waitForFuture(client.requestMeUserInfo(), 5000);
        }
        catch (const KimaiApiError &e)
        {
            spdlog::error("Login failed: {}", e.what());
        }
    }

    spdlog::set_default_logger(previous);

    const auto lines = capture->last_formatted();
    REQUIRE_FALSE(lines.empty());
    for (const auto &line : lines)
    {
        REQUIRE(line.find("secret-api-token-1234") == std::string::npos);
        REQUIRE(line.find("url-password") == std::string::npos);
    }
}

TEST_CASE("KimaiClient - a failed request may release the last client reference", "[client][unit]")
{
    // Answers 401 at once, like a Kimai given a wrong token
    QTcpServer server;
    REQUIRE(server.listen(QHostAddress::LocalHost));
    QObject::connect(&server,
                     &QTcpServer::newConnection,
                     [&server]
                     {
                         auto *socket = server.nextPendingConnection();
                         QObject::connect(socket,
                                          &QTcpSocket::readyRead,
                                          socket,
                                          [socket]
                                          {
                                              socket->readAll();
                                              socket->write("HTTP/1.1 401 Unauthorized\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
                                              socket->disconnectFromHost();
                                          });
                     });

    // Same ownership as KemaiContext::login(): only the continuation keeps the client alive
    QObject context;
    auto    failed = false;
    {
        auto client = std::make_shared<KimaiClient>();
        client->setHost(QString("http://127.0.0.1:%1").arg(server.serverPort()));
        client->setToken("wrong");

        client->requestMeUserInfo()
            .then(&context, [client](const KimaiUser &) {})
            .onFailed(&context, [&failed](const KimaiApiError &)
                      { failed = true; });
    }

    QElapsedTimer elapsed;
    elapsed.start();
    while (!failed && elapsed.elapsed() < 5000)
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
    }
    // Let the deferred deletions run: a use-after-free crashes here
    for (int i = 0; i < 10; ++i)
    {
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QCoreApplication::processEvents();
    }

    REQUIRE(failed);
}

TEST_CASE("KimaiClient - deleteTimeSheet removes a created timesheet", "[client][integration]")
{
    if (host.isEmpty() || token.isEmpty())
    {
        SKIP("Set KEMAI_TEST_HOST and KEMAI_TEST_TOKEN to run integration tests");
    }

    KimaiClient client;
    client.setHost(host);
    client.setToken(token);

    const auto config = waitForFuture(client.requestTimeSheetConfig());
    if (config.trackingMode == KimaiTimeSheetConfig::TrackingMode::Punch)
    {
        SKIP("Punch mode: begin/end cannot be set to create a finished timesheet");
    }

    // Any activity bound to a project of the instance
    const auto activities = waitForFuture(client.requestActivities());
    const auto it         = std::ranges::find_if(activities, [](const KimaiActivity &a)
                                                 { return a.project.has_value(); });
    REQUIRE(it != activities.cend());

    KimaiTimeSheet ts;
    ts.project.id  = it->project->id;
    ts.activity.id = it->id;
    ts.description = QStringLiteral("kemai integration test (deleteTimeSheet)");
    ts.beginAt     = QDateTime(QDate::currentDate().addDays(-1), QTime(3, 0)).toUTC();
    ts.endAt       = ts.beginAt.addSecs(5 * 60);

    const auto created = waitForFuture(client.startTimeSheet(ts, config.trackingMode));
    REQUIRE(created.id > 0);

    REQUIRE_NOTHROW(waitForFuture(client.deleteTimeSheet(created.id)));

    // Gone: deleting it again is a NotFound
    try
    {
        waitForFuture(client.deleteTimeSheet(created.id));
        FAIL("Second delete should fail");
    }
    catch (const KimaiApiError &e)
    {
        REQUIRE(e.kind() == ApiErrorKind::NotFound);
    }
}

TEST_CASE("KimaiClient - a running timesheet stops at a given end or now", "[client][integration]")
{
    if (host.isEmpty() || token.isEmpty())
    {
        SKIP("Set KEMAI_TEST_HOST and KEMAI_TEST_TOKEN to run integration tests");
    }

    KimaiClient client;
    client.setHost(host);
    client.setToken(token);

    // Starting a timer may stop the user's own running one: never touch an account that has one
    if (!waitForFuture(client.requestActiveTimeSheets()).isEmpty())
    {
        SKIP("A timer is running on this account");
    }

    const auto config = waitForFuture(client.requestTimeSheetConfig());
    if (config.trackingMode == KimaiTimeSheetConfig::TrackingMode::Punch)
    {
        SKIP("Punch mode: begin cannot be set");
    }

    const auto activities = waitForFuture(client.requestActivities());
    const auto it         = std::ranges::find_if(activities, [](const KimaiActivity &a)
                                                 { return a.project.has_value(); });
    REQUIRE(it != activities.cend());

    KimaiTimeSheet ts;
    ts.project.id  = it->project->id;
    ts.activity.id = it->id;
    ts.description = QStringLiteral("kemai integration test (stop)");
    ts.beginAt     = QDateTime::currentDateTimeUtc().addSecs(-10 * 60);

    // Backdated: the end the user left at (server rounding rules may move it by up to a minute)
    const auto running = waitForFuture(client.startTimeSheet(ts, config.trackingMode));
    REQUIRE(running.id > 0);
    REQUIRE_FALSE(running.endAt.isValid());

    auto       backdated = running;
    const auto left      = running.beginAt.addSecs(60);
    backdated.endAt      = left;
    const auto ended     = waitForFuture(client.updateTimeSheet(backdated, config.trackingMode));
    REQUIRE(ended.endAt.isValid());
    REQUIRE(std::abs(ended.endAt.secsTo(left)) <= 60);
    REQUIRE(waitForFuture(client.requestActiveTimeSheets()).isEmpty());
    REQUIRE_NOTHROW(waitForFuture(client.deleteTimeSheet(running.id)));

    // Plain stop: ends now
    const auto running2 = waitForFuture(client.startTimeSheet(ts, config.trackingMode));
    const auto stopped  = waitForFuture(client.stopTimeSheet(running2.id));
    REQUIRE(stopped.endAt.isValid());
    REQUIRE(std::abs(stopped.endAt.secsTo(QDateTime::currentDateTimeUtc())) <= 120);
    REQUIRE_NOTHROW(waitForFuture(client.deleteTimeSheet(running2.id)));
}

TEST_CASE("KimaiClient::validationErrorFromJson - field and form errors", "[client][unit]")
{
    const auto fieldError = QJsonDocument::fromJson(
        R"({"code":400,"message":"Validation Failed","errors":{"children":{"name":{},"country":{"errors":["Please select a valid country."]}}}})");
    REQUIRE(KimaiClient::validationErrorFromJson(fieldError) == std::pair<QString, QString>{"country", "Please select a valid country."});

    const auto formError = QJsonDocument::fromJson(
        R"({"code":400,"message":"Validation Failed","errors":{"errors":["This form should not contain extra fields."],"children":{"name":{}}}})");
    REQUIRE(KimaiClient::validationErrorFromJson(formError) == std::pair<QString, QString>{"", "This form should not contain extra fields."});

    REQUIRE(KimaiClient::validationErrorFromJson(QJsonDocument::fromJson(R"({"message":"Not found"})")).second.isEmpty());
}

TEST_CASE("KimaiClient - creates a customer, project and activity", "[client][integration]")
{
    if (host.isEmpty() || token.isEmpty())
    {
        SKIP("Set KEMAI_TEST_HOST and KEMAI_TEST_TOKEN to run integration tests");
    }

    KimaiClient client;
    client.setHost(host);
    client.setToken(token);

    // The client has no delete for these: cleanup goes straight to the API, newest first
    QNetworkAccessManager network;
    const auto            remove = [&](const QString &path)
    {
        QNetworkRequest request(QUrl(host + "/api/" + path));
        request.setRawHeader("Authorization", QString("Bearer %1").arg(token).toLatin1());
        auto      *reply = network.deleteResource(request);
        QEventLoop loop;
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();
        reply->deleteLater();
    };

    KimaiCustomer customer;
    customer.name              = QStringLiteral("kemai integration test customer");
    customer.countryKey        = "FR";
    customer.currencyKey       = "EUR";
    customer.timezone          = "Europe/Paris";
    customer.language          = "fr";
    const auto createdCustomer = waitForFuture(client.addCustomer(customer));
    REQUIRE(createdCustomer.id > 0);
    REQUIRE(createdCustomer.name == customer.name);
    REQUIRE(createdCustomer.language == "fr");

    KimaiProject project;
    project.name              = QStringLiteral("kemai integration test project");
    project.customer.id       = createdCustomer.id;
    const auto createdProject = waitForFuture(client.addProject(project));
    REQUIRE(createdProject.id > 0);

    KimaiActivity activity;
    activity.name              = QStringLiteral("kemai integration test activity");
    activity.project           = createdProject;
    const auto createdActivity = waitForFuture(client.addActivity(activity));
    REQUIRE(createdActivity.id > 0);
    REQUIRE(createdActivity.project.has_value());
    REQUIRE(createdActivity.project->id == createdProject.id);

    // An invalid value comes back with Kimai's field message
    auto invalid       = customer;
    invalid.countryKey = "XX";
    try
    {
        const auto unexpected = waitForFuture(client.addCustomer(invalid));
        remove(QString("customers/%1").arg(unexpected.id));
        FAIL("An invalid country should be refused");
    }
    catch (const KimaiApiError &e)
    {
        REQUIRE(e.kind() == ApiErrorKind::InvalidValue);
        REQUIRE(e.field() == "country");
        REQUIRE_FALSE(e.serverMessage().isEmpty());
    }

    remove(QString("activities/%1").arg(createdActivity.id));
    remove(QString("projects/%1").arg(createdProject.id));
    remove(QString("customers/%1").arg(createdCustomer.id));
}

TEST_CASE("KimaiClient::normalizedHost - https by default, no trailing slash", "[client][unit]")
{
    REQUIRE(KimaiClient::normalizedHost("demo.kimai.org") == "https://demo.kimai.org");
    REQUIRE(KimaiClient::normalizedHost("  kimai.example.lan:8001/kimai/ ") == "https://kimai.example.lan:8001/kimai");
    REQUIRE(KimaiClient::normalizedHost("https://demo.kimai.org/") == "https://demo.kimai.org");
    REQUIRE(KimaiClient::normalizedHost("http://192.168.1.40:8001") == "http://192.168.1.40:8001"); // explicit http is kept
    REQUIRE(KimaiClient::normalizedHost("").isEmpty());
}

TEST_CASE("KimaiClient - an http host redirecting to https fails as HttpsRedirect", "[client][unit]")
{
    // The https side does not need to answer: only the redirect matters
    QTcpServer httpsSide;
    REQUIRE(httpsSide.listen(QHostAddress::LocalHost));
    const auto httpsBase = QString("https://127.0.0.1:%1/kimai").arg(httpsSide.serverPort());

    QTcpServer httpSide;
    REQUIRE(httpSide.listen(QHostAddress::LocalHost));
    QObject::connect(&httpSide,
                     &QTcpServer::newConnection,
                     [&]
                     {
                         auto *socket = httpSide.nextPendingConnection();
                         QObject::connect(socket,
                                          &QTcpSocket::readyRead,
                                          socket,
                                          [socket, httpsBase]
                                          {
                                              socket->readAll();
                                              socket->write(QString("HTTP/1.1 301 Moved Permanently\r\nLocation: %1/api/users/me\r\nContent-Length: 0\r\n"
                                                                    "Connection: close\r\n\r\n")
                                                                .arg(httpsBase)
                                                                .toLatin1());
                                              socket->disconnectFromHost();
                                          });
                     });

    KimaiClient client;
    client.setHost(QString("http://127.0.0.1:%1/kimai").arg(httpSide.serverPort()));
    client.setToken("token");
    client.setTransferTimeout(std::chrono::milliseconds(2000));

    try
    {
        waitForFuture(client.requestMeUserInfo(), 5000);
        FAIL("The redirect should fail the request");
    }
    catch (const KimaiApiError &e)
    {
        REQUIRE(e.kind() == ApiErrorKind::HttpsRedirect);
        REQUIRE(e.serverMessage() == httpsBase);
    }
}

// Shape of GET config/colors (demo.kimai.org 2026-10-28, Kimai's default palette)
TEST_CASE("KimaiClient::colorChoicesFromJson - the palette by name", "[client][unit]")
{
    const auto doc = QJsonDocument::fromJson(R"({"Silver":"#c0c0c0","Lavender":"#E6E6FA","Broken":null})");
    REQUIRE(KimaiClient::colorChoicesFromJson(doc) == KimaiColorChoices{
                                                          {.name = "Lavender", .color = "#E6E6FA"},
                                                          {.name = "Silver",   .color = "#c0c0c0"}
    });
    REQUIRE_THROWS(KimaiClient::colorChoicesFromJson(QJsonDocument::fromJson("[]")));
}

TEST_CASE("KimaiClient::tagNamesFromJson - accepts tag objects and plain names", "[client][unit]")
{
    const auto objects = QJsonDocument::fromJson(R"([{"id": 1, "name": "QML", "color": null}, {"id": 2, "name": "Frontend"}])");
    REQUIRE(KimaiClient::tagNamesFromJson(objects) == QStringList{"QML", "Frontend"});

    const auto names = QJsonDocument::fromJson(R"(["QML", "Frontend"])");
    REQUIRE(KimaiClient::tagNamesFromJson(names) == QStringList{"QML", "Frontend"});

    REQUIRE_THROWS_AS(KimaiClient::tagNamesFromJson(QJsonDocument::fromJson(R"({"name": "QML"})")), std::runtime_error);
}

TEST_CASE("KimaiClient - requestTags finds existing tags by name", "[client][integration]")
{
    if (host.isEmpty() || token.isEmpty())
    {
        SKIP("Set KEMAI_TEST_HOST and KEMAI_TEST_TOKEN to run integration tests");
    }

    KimaiClient client;
    client.setHost(host);
    client.setToken(token);

    // Any tag of the instance, then search it back by name
    const auto allTags = waitForFuture(client.requestTags(QString()));
    REQUIRE_FALSE(allTags.isEmpty());

    const auto found = waitForFuture(client.requestTags(allTags.first()));
    REQUIRE(found.contains(allTags.first()));
}

TEST_CASE("KimaiMetaFieldRule::fromJson - definition of a custom field", "[client][unit][customFields]")
{
    // Shape of GET metafields?entity=timesheet (MetaFieldsBundle, demo.kimai.org 2026-10-27)
    const auto json = QJsonDocument::fromJson(R"([
        {"id": 3, "name": "ticket", "label": "Ticket", "help": "Issue number", "value": null, "type": "text",
         "visible": true, "required": true, "entityType": "timesheet", "customer": null, "project": 120, "activity": null},
        {"id": 4, "name": "kind", "label": "Kind", "help": null, "value": ",dev|Development,ops|Operations",
         "type": "choice", "visible": true, "required": false, "entityType": "timesheet", "customer": 5, "project": null, "activity": 9}
    ])")
                          .array();

    const auto ticket = KimaiMetaFieldRule::fromJson(json.at(0));
    CHECK(ticket.name == "ticket");
    CHECK(ticket.label == "Ticket");
    CHECK(ticket.help == "Issue number");
    CHECK(ticket.type == "text");
    CHECK(ticket.rawValue.isEmpty());
    CHECK(ticket.required);
    CHECK(ticket.customerId == 0);
    CHECK(ticket.projectId == 120);
    CHECK(ticket.activityId == 0);

    const auto kind = KimaiMetaFieldRule::fromJson(json.at(1));
    CHECK(kind.help.isEmpty());
    CHECK(kind.rawValue == ",dev|Development,ops|Operations");
    CHECK_FALSE(kind.required);
    CHECK(kind.customerId == 5);
    CHECK(kind.activityId == 9);

    CHECK_THROWS(KimaiMetaFieldRule::fromJson(QJsonValue(QStringLiteral("not an object"))));
}

TEST_CASE("KimaiClient::setTimeSheetMeta - PATCH timesheets/{id}/meta with name and value", "[client][unit][customFields]")
{
    // Captures one request and answers with the updated entry
    QTcpServer server;
    REQUIRE(server.listen(QHostAddress::LocalHost));
    QByteArray request;
    QObject::connect(&server,
                     &QTcpServer::newConnection,
                     &server,
                     [&server, &request]()
                     {
                         auto *socket = server.nextPendingConnection();
                         QObject::connect(socket,
                                          &QTcpSocket::readyRead,
                                          socket,
                                          [socket, &request]()
                                          {
                                              request += socket->readAll();
                                              const auto headerEnd = request.indexOf("\r\n\r\n");
                                              if (headerEnd < 0)
                                              {
                                                  return;
                                              }
                                              const auto lengthAt = request.toLower().indexOf("content-length:");
                                              const auto length   = request.mid(lengthAt + 15, request.indexOf("\r\n", lengthAt) - lengthAt - 15).trimmed().toInt();
                                              if (request.size() < headerEnd + 4 + length)
                                              {
                                                  return;
                                              }
                                              const QByteArray body = R"({"id": 42, "begin": "2026-10-28T09:00:00+0100", "project": 1, "activity": 2, "metaFields": [{"name": "ticket", "value": "KEM-1"}]})";
                                              socket->write("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " + QByteArray::number(body.size()) +
                                                            "\r\nConnection: close\r\n\r\n" + body);
                                              socket->disconnectFromHost();
                                          });
                     });

    KimaiClient client;
    client.setHost(QString("http://127.0.0.1:%1").arg(server.serverPort()));
    client.setToken("token");

    const auto updated = waitForFuture(client.setTimeSheetMeta(42, "ticket", "KEM-1"), 5000);
    CHECK(updated.id == 42);
    CHECK(updated.metaFields.value("ticket") == "KEM-1");

    REQUIRE(request.startsWith("PATCH /api/timesheets/42/meta "));
    const auto body = QJsonDocument::fromJson(request.mid(request.indexOf("\r\n\r\n") + 4)).object();
    CHECK(body == QJsonObject{
                      {"name",  "ticket"},
                      {"value", "KEM-1" }
    });
}

TEST_CASE("KimaiClient::requestTimeSheets - period query with begin, end, page size and page", "[client][unit]")
{
    // Captures one request and answers with an empty page
    QTcpServer server;
    REQUIRE(server.listen(QHostAddress::LocalHost));
    QByteArray request;
    QObject::connect(&server,
                     &QTcpServer::newConnection,
                     &server,
                     [&server, &request]()
                     {
                         auto *socket = server.nextPendingConnection();
                         QObject::connect(socket,
                                          &QTcpSocket::readyRead,
                                          socket,
                                          [socket, &request]()
                                          {
                                              request += socket->readAll();
                                              if (!request.contains("\r\n\r\n"))
                                              {
                                                  return;
                                              }
                                              socket->write("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: 2\r\nConnection: close\r\n\r\n[]");
                                              socket->disconnectFromHost();
                                          });
                     });

    KimaiClient client;
    client.setHost(QString("http://127.0.0.1:%1").arg(server.serverPort()));
    client.setToken("token");

    const QDateTime begin(QDate(2026, 10, 25), QTime(0, 0));
    const QDateTime end(QDate(2026, 11, 2), QTime(23, 59, 59));
    const auto      sheets = waitForFuture(client.requestTimeSheets(begin, end, 2), 5000);
    CHECK(sheets.isEmpty());

    const auto requestLine = request.left(request.indexOf("\r\n"));
    REQUIRE(requestLine.startsWith("GET /api/timesheets?"));
    const QUrlQuery query(QUrl::fromPercentEncoding(requestLine.mid(4, requestLine.lastIndexOf(' ') - 4)).section('?', 1));
    CHECK(query.queryItemValue("begin") == "2026-10-25T00:00:00");
    CHECK(query.queryItemValue("end") == "2026-11-02T23:59:59");
    CHECK(query.queryItemValue("size") == "500");
    CHECK(query.queryItemValue("page") == "2");
    CHECK(query.queryItemValue("full") == "true");
}

TEST_CASE("KimaiClient::setTimeSheetMeta - sets then reads a value on the test instance", "[client][integration][customFields]")
{
    // Needs an instance with the custom fields plugin and a text field of time entries named by KEMAI_TEST_META_FIELD
    const auto field = qEnvironmentVariable("KEMAI_TEST_META_FIELD");
    if (host.isEmpty() || token.isEmpty() || field.isEmpty())
    {
        SKIP("Set KEMAI_TEST_HOST, KEMAI_TEST_TOKEN and KEMAI_TEST_META_FIELD to run this integration test");
    }

    KimaiClient client;
    client.setHost(host);
    client.setToken(token);

    const auto activities = waitForFuture(client.requestActivities());
    const auto it         = std::ranges::find_if(activities, [](const KimaiActivity &a)
                                                 { return a.project.has_value(); });
    REQUIRE(it != activities.end());

    KimaiTimeSheet ts;
    ts.project         = *it->project;
    ts.activity        = *it;
    ts.beginAt         = QDateTime::currentDateTime().addSecs(-3600);
    ts.endAt           = QDateTime::currentDateTime().addSecs(-1800);
    ts.description     = QStringLiteral("kemai integration test (setTimeSheetMeta)");
    const auto created = waitForFuture(client.startTimeSheet(ts, KimaiTimeSheetConfig::TrackingMode::Default));
    REQUIRE(created.id > 0);

    const auto value   = QStringLiteral("kemai-%1").arg(QDateTime::currentSecsSinceEpoch());
    const auto updated = waitForFuture(client.setTimeSheetMeta(created.id, field, value));
    CHECK(updated.metaFields.value(field) == value);

    REQUIRE_NOTHROW(waitForFuture(client.deleteTimeSheet(created.id)));
}
