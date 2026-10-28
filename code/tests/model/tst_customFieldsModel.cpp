// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include <catch2/catch_test_macros.hpp>

#include <model/customFieldsModel.h>

using namespace kemai;

namespace
{
    KimaiMetaFieldRule rule(const QString &name, const QString &type, int projectId = 0, const QString &rawValue = {})
    {
        return {.name = name, .type = type, .rawValue = rawValue, .required = true, .projectId = projectId};
    }

    QVariant role(const CustomFieldsModel &model, int row, CustomFieldsModel::Role r)
    {
        return model.data(model.index(row), static_cast<int>(r));
    }

    using Role = CustomFieldsModel::Role;
} // namespace

TEST_CASE("CustomFields::parseInput and inputText - typed values and stored formats", "[customFields][unit]")
{
    const QLocale french(QLocale::French, QLocale::France);
    const QLocale english(QLocale::English, QLocale::UnitedStates);
    using Error = CustomFields::Error;

    SECTION("Durations")
    {
        REQUIRE(CustomFields::parseInput("duration", "345:00", english).stored == "1242000");
        REQUIRE(CustomFields::parseInput("duration", "1:30", english).stored == "5400");
        REQUIRE(CustomFields::parseInput("duration", "2", english).stored == "7200");
        REQUIRE(CustomFields::parseInput("duration", "1,5", french).stored == "5400");
        REQUIRE(CustomFields::parseInput("duration", "1:75", english).error == Error::MinutesOutOfRange);
        REQUIRE(CustomFields::parseInput("duration", "soon", english).error == Error::NotDuration);
        REQUIRE(CustomFields::parseInput("duration", " ", english).stored.isEmpty());
        REQUIRE(CustomFields::inputText("duration", "1242000", english) == "345:00");
        REQUIRE(CustomFields::inputText("duration", "300", english) == "0:05");
    }

    SECTION("Numbers and amounts take the locale's separator or a dot")
    {
        REQUIRE(CustomFields::parseInput("number", "4,123", french).stored == "4.123");
        REQUIRE(CustomFields::parseInput("number", "4.123", french).stored == "4.123");
        REQUIRE(CustomFields::parseInput("money", "12.5", english).stored == "12.5");
        REQUIRE(CustomFields::inputText("number", "4.123", french) == "4,123");
        REQUIRE(CustomFields::inputText("money", "123", english) == "123.00");
        REQUIRE(CustomFields::inputText("money", "1234", french) == "1234,00");
        REQUIRE(CustomFields::inputText("money", "12.5", english) == "12.5");
    }

    SECTION("Other types are stored as typed")
    {
        REQUIRE(CustomFields::parseInput("text", "  KEM-12 ", english).stored == "  KEM-12 ");
        REQUIRE(CustomFields::inputText("integer", "1231", french) == "1231");
    }

    SECTION("Date-times are stored in the user's zone")
    {
        const QTimeZone berlin("Europe/Berlin");
        const auto      local = QDateTime(QDate(2026, 10, 22), QTime(8, 30), berlin).toLocalTime();
        REQUIRE(CustomFields::storedDateTime(local, berlin) == "2026-10-22 08:30:00");
        REQUIRE(CustomFields::dateTimeFromStored("2026-10-22 08:30:00", berlin) == local);
        REQUIRE_FALSE(CustomFields::dateTimeFromStored("2026-10-22", berlin).isValid());
    }
}

TEST_CASE("CustomFieldsModel - rows, values and states", "[customFields][model][unit]")
{
    CustomFieldsModel model;
    model.setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
    model.setRules({rule("ticket", "text"), rule("hours", "duration"), rule("kind", "choice", 0, "a|Alpha,b|Beta"), rule("template", "invoice-template"), rule("site", "text", 9)});
    model.load(1, 2, 3, {
                            {"ticket", "KEM-1"}
    },
               "EUR");

    REQUIRE(model.rowCount() == 4);
    REQUIRE(model.currency() == "EUR");
    REQUIRE(role(model, 0, Role::Label) == "ticket");
    REQUIRE(role(model, 2, Role::Value) == "a");
    REQUIRE(role(model, 2, Role::Choices).toList().size() == 2);
    REQUIRE(role(model, 3, Role::State) == "unsupported");
    REQUIRE(role(model, 1, Role::State) == "missing");
    REQUIRE(model.missingCount() == 1);
    REQUIRE_FALSE(model.isValid());
    REQUIRE(model.firstIncompleteRow() == 1);

    SECTION("Typed text that does not parse is invalid, with the parse error")
    {
        model.setInput(1, "1:75");
        REQUIRE(role(model, 1, Role::State) == "invalid");
        REQUIRE(role(model, 1, Role::Error) == "MinutesOutOfRange");
        REQUIRE(model.invalidCount() == 1);
        REQUIRE(model.missingCount() == 0);

        model.setInput(1, "1:15");
        REQUIRE(role(model, 1, Role::State) == "ok");
        REQUIRE(role(model, 1, Role::Value) == "4500");
        REQUIRE(model.isValid());
        REQUIRE(model.values() == QMap<QString, QString>{
                                      {"ticket", "KEM-1"},
                                      {"hours",  "4500" },
                                      {"kind",   "a"    }
        });
    }

    SECTION("Clearing a field with a default leaves it missing")
    {
        model.setValue(2, "");
        REQUIRE(role(model, 2, Role::State) == "missing");
    }

    SECTION("Moving the entry keeps drafts and marks the new fields")
    {
        model.setInput(0, "KEM-2");
        model.moveTo(1, 9, 3, "USD");
        REQUIRE(model.rowCount() == 5);
        REQUIRE(role(model, 0, Role::Value) == "KEM-2");
        REQUIRE_FALSE(role(model, 0, Role::Appeared).toBool());
        REQUIRE(role(model, 4, Role::Name) == "site");
        REQUIRE(role(model, 4, Role::Appeared).toBool());
        REQUIRE(model.currency() == "USD");

        model.moveTo(1, 2, 3, "EUR");
        REQUIRE(model.rowCount() == 4);
        REQUIRE_FALSE(model.values().contains("site"));
    }

    SECTION("Unsaved marks until the next load")
    {
        model.markUnsaved({"kind"});
        REQUIRE(role(model, 2, Role::Unsaved).toBool());
        REQUIRE_FALSE(role(model, 0, Role::Unsaved).toBool());
        model.load(1, 2, 3, {}, {});
        REQUIRE_FALSE(role(model, 2, Role::Unsaved).toBool());
    }

    SECTION("State changes are signalled")
    {
        int states = 0;
        QObject::connect(&model, &CustomFieldsModel::statesChanged, [&states]
                         { ++states; });
        model.setInput(1, "2");
        REQUIRE(states == 1);
        model.setInput(1, "2:00");
        REQUIRE(states == 1); // same stored value
    }
}
