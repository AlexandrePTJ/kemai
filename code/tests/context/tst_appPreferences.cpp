// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include <catch2/catch_test_macros.hpp>

#include <QTimeZone>

#include "appPreferences.h"

using namespace kemai;

TEST_CASE("AppPreferences - update preferences and their defaults", "[preferences][unit]")
{
    AppPreferences preferences;
    REQUIRE(preferences.checkUpdatesAtStart());
    REQUIRE(preferences.ignoredVersion().isEmpty());
    REQUIRE_FALSE(preferences.lastUpdateCheck().isValid());

    int changes = 0;
    QObject::connect(&preferences, &AppPreferences::changed, [&changes]
                     { ++changes; });
    preferences.setCheckUpdatesAtStart(false);
    preferences.setIgnoredVersion("2026.12.1");
    const auto when = QDateTime(QDate(2026, 10, 25), QTime(8, 42), QTimeZone::UTC);
    preferences.setLastUpdateCheck(when);
    REQUIRE(changes == 3);

    // Written back whole and read again by another instance (as after a restart)
    AppPreferences reread;
    reread.setJson(preferences.json());
    REQUIRE_FALSE(reread.checkUpdatesAtStart());
    REQUIRE(reread.ignoredVersion() == "2026.12.1");
    REQUIRE(reread.lastUpdateCheck() == when);

    // Same value: nothing to save
    reread.setIgnoredVersion("2026.12.1");
    bool saved = false;
    QObject::connect(&reread, &AppPreferences::changed, [&saved]
                     { saved = true; });
    reread.setIgnoredVersion("2026.12.1");
    REQUIRE_FALSE(saved);
}

TEST_CASE("AppPreferences - tray preferences are off by default and kept", "[preferences][unit]")
{
    AppPreferences preferences;
    REQUIRE_FALSE(preferences.closeToTray());
    REQUIRE_FALSE(preferences.minimizeToTray());

    preferences.setCloseToTray(true);
    preferences.setMinimizeToTray(true);

    AppPreferences reread;
    reread.setJson(preferences.json());
    REQUIRE(reread.closeToTray());
    REQUIRE(reread.minimizeToTray());
}

TEST_CASE("AppPreferences - keys of a newer Kemai survive", "[preferences][unit]")
{
    AppPreferences preferences;
    preferences.setJson(QJsonObject{
        {"futureKey", 42}
    });
    preferences.setIgnoredVersion("2026.12.1");
    REQUIRE(preferences.json().value("futureKey").toInt() == 42);
}

TEST_CASE("AppPreferences - the 0.11 import is offered until declined for good", "[preferences][unit]")
{
    AppPreferences preferences;
    REQUIRE(preferences.legacyImportOffer());

    preferences.setLegacyImportOffer(false);

    AppPreferences reread;
    reread.setJson(preferences.json());
    REQUIRE_FALSE(reread.legacyImportOffer());
}
