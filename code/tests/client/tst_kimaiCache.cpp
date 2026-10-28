// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include <catch2/catch_test_macros.hpp>

#include <QCoreApplication>

#include "../mocks/fakeKimaiClient.h"
#include <client/kimaiCache.h>

using namespace kemai;

namespace
{
    void processEvents(int rounds = 10)
    {
        for (int i = 0; i < rounds; ++i)
        {
            QCoreApplication::processEvents();
        }
    }

    FakeKimaiClient makeFakeWithResources()
    {
        FakeKimaiClient fake;

        KimaiCustomer customer;
        customer.id   = 5;
        customer.name = "ACME";
        fake.customers.append(customer);

        KimaiProject project;
        project.id          = 7;
        project.name        = "Rocket";
        project.customer.id = 5; // name resolved by consolidation
        fake.projects.append(project);

        KimaiActivity boundActivity;
        boundActivity.id          = 11;
        boundActivity.name        = "Design";
        boundActivity.project     = KimaiProject{};
        boundActivity.project->id = 7;
        fake.activities.append(boundActivity);

        KimaiActivity globalActivity;
        globalActivity.id   = 12;
        globalActivity.name = "Meeting";
        fake.activities.append(globalActivity);

        fake.timesheetConfig.trackingMode = KimaiTimeSheetConfig::TrackingMode::Punch;

        return fake;
    }
} // namespace

TEST_CASE("KimaiCache - nominal load consolidates and emits loaded", "[cache][unit]")
{
    auto       fake = makeFakeWithResources();
    KimaiCache cache;

    int loadedCount = 0;
    int failedCount = 0;
    QObject::connect(&cache, &KimaiCache::loaded, [&loadedCount]
                     { ++loadedCount; });
    QObject::connect(&cache, &KimaiCache::loadFailed, [&failedCount]
                     { ++failedCount; });

    cache.load(&fake);
    processEvents();

    REQUIRE(loadedCount == 1);
    REQUIRE(failedCount == 0);
    REQUIRE_FALSE(cache.lastError().has_value());

    // Project customer resolved from customers list
    REQUIRE(cache.projects().size() == 1);
    REQUIRE(cache.projects().first().customer.name == "ACME");

    // Bound activity resolved + global activity fanned out to every project
    REQUIRE(cache.activities().size() == 2);
    for (const auto &activity : cache.activities())
    {
        REQUIRE(activity.project.has_value());
        REQUIRE(activity.project->name == "Rocket");
        REQUIRE(activity.global == (activity.id == 12));
    }

    REQUIRE(cache.timesheetConfig().trackingMode == KimaiTimeSheetConfig::TrackingMode::Punch);
}

TEST_CASE("KimaiCache - any resource failure fails the whole load", "[cache][unit]")
{
    auto fake = makeFakeWithResources();
    fake.failWith("requestActivities", ApiErrorKind::ServerError, 500);

    KimaiCache cache;

    int loadedCount = 0;
    int failedCount = 0;
    QObject::connect(&cache, &KimaiCache::loaded, [&loadedCount]
                     { ++loadedCount; });
    QObject::connect(&cache, &KimaiCache::loadFailed, [&failedCount]
                     { ++failedCount; });

    cache.load(&fake);
    processEvents();

    REQUIRE(loadedCount == 0);
    REQUIRE(failedCount == 1);
    REQUIRE(cache.lastError().has_value());
    REQUIRE(cache.lastError()->kind() == ApiErrorKind::ServerError);
    REQUIRE(cache.lastError()->httpStatus() == 500);
}

TEST_CASE("KimaiCache - failed refresh keeps the previous complete snapshot", "[cache][unit]")
{
    auto       fake = makeFakeWithResources();
    KimaiCache cache;

    cache.load(&fake);
    processEvents();
    REQUIRE(cache.hasData());
    const auto firstLoadedAt = cache.lastLoadedAt();

    int failedCount = 0;
    QObject::connect(&cache, &KimaiCache::loadFailed, [&failedCount]
                     { ++failedCount; });

    // New data on the server, but one resource fails: nothing of it must leak in
    fake.customers.first().name = "Changed";
    fake.failWith("requestActivities", ApiErrorKind::NoNetwork);

    cache.load(&fake);
    processEvents();

    REQUIRE(failedCount == 1);
    REQUIRE(cache.hasData());
    REQUIRE(cache.lastLoadedAt() == firstLoadedAt);
    REQUIRE(cache.lastError()->kind() == ApiErrorKind::NoNetwork);
    REQUIRE(cache.customers().first().name == "ACME");
    REQUIRE(cache.projects().first().customer.name == "ACME");
    REQUIRE(cache.activities().size() == 2);
}

TEST_CASE("KimaiCache - successful refresh replaces the snapshot and clears the error", "[cache][unit]")
{
    auto       fake = makeFakeWithResources();
    KimaiCache cache;

    fake.failWith("requestCustomers", ApiErrorKind::ServerError, 500);
    cache.load(&fake);
    processEvents();
    REQUIRE_FALSE(cache.hasData());
    REQUIRE(cache.lastError().has_value());

    fake.errors.clear();
    fake.customers.first().name = "Changed";
    cache.load(&fake);
    processEvents();

    REQUIRE(cache.hasData());
    REQUIRE_FALSE(cache.lastError().has_value());
    REQUIRE(cache.projects().first().customer.name == "Changed");
}

TEST_CASE("KimaiCache - a new load supersedes the one in flight", "[cache][unit]")
{
    auto       fake = makeFakeWithResources();
    KimaiCache cache;

    int loadedCount = 0;
    int failedCount = 0;
    QObject::connect(&cache, &KimaiCache::loaded, [&loadedCount]
                     { ++loadedCount; });
    QObject::connect(&cache, &KimaiCache::loadFailed, [&failedCount]
                     { ++failedCount; });

    // First load would fail; it is superseded before any response arrives
    fake.failWith("requestProjects", ApiErrorKind::ServerError, 500);
    cache.load(&fake);
    REQUIRE(cache.isLoading());

    fake.errors.clear();
    fake.customers.first().name = "Second";
    cache.load(&fake);
    processEvents();

    REQUIRE(loadedCount == 1);
    REQUIRE(failedCount == 0);
    REQUIRE_FALSE(cache.isLoading());
    REQUIRE(cache.customers().first().name == "Second");
}

TEST_CASE("KimaiCache - custom field definitions: required ones kept, none without the plugin", "[cache][unit][customFields]")
{
    SECTION("required definitions are kept")
    {
        auto fake = makeFakeWithResources();
        fake.metaFieldRules.append(KimaiMetaFieldRule{.name = "ticket", .type = "text", .required = true});
        fake.metaFieldRules.append(KimaiMetaFieldRule{.name = "note", .type = "text", .required = false});

        KimaiCache cache;
        cache.load(&fake);
        processEvents();

        REQUIRE(cache.hasData());
        REQUIRE(cache.timesheetMetaFields().size() == 1);
        CHECK(cache.timesheetMetaFields().first().name == "ticket");
    }

    SECTION("404: no plugin, the load still succeeds with no field")
    {
        auto fake = makeFakeWithResources();
        fake.failWith("requestTimeSheetMetaFields", ApiErrorKind::NotFound, 404);

        KimaiCache cache;
        int        failedCount = 0;
        QObject::connect(&cache, &KimaiCache::loadFailed, [&failedCount]
                         { ++failedCount; });
        cache.load(&fake);
        processEvents();

        CHECK(failedCount == 0);
        REQUIRE(cache.hasData());
        CHECK(cache.timesheetMetaFields().isEmpty());
    }

    SECTION("any other failure fails the whole load")
    {
        auto fake = makeFakeWithResources();
        fake.failWith("requestTimeSheetMetaFields", ApiErrorKind::ServerError, 500);

        KimaiCache cache;
        cache.load(&fake);
        processEvents();

        CHECK_FALSE(cache.hasData());
        REQUIRE(cache.lastError().has_value());
        CHECK(cache.lastError()->kind() == ApiErrorKind::ServerError);
    }

    SECTION("the palette is loaded only with a color field")
    {
        auto fake         = makeFakeWithResources();
        fake.colorChoices = {
            {.name = "Silver", .color = "#c0c0c0"}
        };
        fake.metaFieldRules.append(KimaiMetaFieldRule{.name = "ticket", .type = "text", .required = true});

        KimaiCache cache;
        cache.load(&fake);
        processEvents();
        CHECK_FALSE(fake.calls.contains("requestColorChoices"));
        CHECK(cache.colorChoices().isEmpty());

        fake.metaFieldRules.append(KimaiMetaFieldRule{.name = "label", .type = "color", .required = true});
        cache.load(&fake);
        processEvents();
        REQUIRE(cache.hasData());
        CHECK(cache.colorChoices() == KimaiColorChoices{
                                          {.name = "Silver", .color = "#c0c0c0"}
        });
    }

    SECTION("a palette failure fails the load, except 404")
    {
        auto fake = makeFakeWithResources();
        fake.metaFieldRules.append(KimaiMetaFieldRule{.name = "label", .type = "color-free", .required = true});

        fake.failWith("requestColorChoices", ApiErrorKind::NotFound, 404);
        KimaiCache cache;
        cache.load(&fake);
        processEvents();
        REQUIRE(cache.hasData());
        CHECK(cache.colorChoices().isEmpty());

        fake.failWith("requestColorChoices", ApiErrorKind::ServerError, 500);
        KimaiCache failing;
        failing.load(&fake);
        processEvents();
        CHECK_FALSE(failing.hasData());
    }
}
