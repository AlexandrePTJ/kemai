// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include <catch2/catch_test_macros.hpp>

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>

#include <misc/customFields.h>

using namespace kemai;

namespace
{
    KimaiMetaFieldRule requiredRule(const QString &name, const QString &type, const QString &rawValue = {})
    {
        return {.name = name, .label = name, .type = type, .rawValue = rawValue, .required = true};
    }

    const CustomFields::KnownCodes Codes{
        .countries  = {"FR", "DE"},
        .currencies = {"EUR", "USD"},
        .languages  = {"en", "fr", "pt_BR"}
    };

    CustomFields::Field resolveOne(const KimaiMetaFieldRule &rule, const QString &value)
    {
        const auto fields = CustomFields::resolve({
                                                      rule
        },
                                                  1,
                                                  2,
                                                  3,
                                                  {{rule.name, value}},
                                                  Codes);
        REQUIRE(fields.size() == 1);
        return fields.first();
    }
} // namespace

TEST_CASE("CustomFields::applies - every restriction set must match", "[customFields][unit]")
{
    auto rule = requiredRule("ticket", "text");
    REQUIRE(CustomFields::applies(rule, 1, 2, 3));

    rule.projectId = 120;
    REQUIRE(CustomFields::applies(rule, 1, 120, 3));
    REQUIRE_FALSE(CustomFields::applies(rule, 1, 2, 3));

    rule.activityId = 7;
    REQUIRE(CustomFields::applies(rule, 1, 120, 7));
    REQUIRE_FALSE(CustomFields::applies(rule, 1, 120, 3));

    rule            = requiredRule("ticket", "text");
    rule.customerId = 5;
    REQUIRE(CustomFields::applies(rule, 5, 2, 3));
    REQUIRE_FALSE(CustomFields::applies(rule, 1, 2, 3));

    SECTION("Neither the customer alone nor a global activity alone is enough (checked on the demo)")
    {
        rule.activityId = 7;
        REQUIRE(CustomFields::applies(rule, 5, 2, 7));
        REQUIRE_FALSE(CustomFields::applies(rule, 5, 2, 3));
        REQUIRE_FALSE(CustomFields::applies(rule, 1, 2, 7));
    }
}

TEST_CASE("CustomFields::resolve - only required fields that apply, in Kimai's order", "[customFields][unit]")
{
    auto optional          = requiredRule("note", "text");
    optional.required      = false;
    auto otherProject      = requiredRule("ticket", "text");
    otherProject.projectId = 99;

    const auto fields = CustomFields::resolve({requiredRule("b", "text"), optional, otherProject, requiredRule("a", "text")}, 1, 2, 3, {}, Codes);

    REQUIRE(fields.size() == 2);
    REQUIRE(fields[0].rule.name == "b");
    REQUIRE(fields[1].rule.name == "a");
}

TEST_CASE("CustomFields::choices - options and their labels", "[customFields][unit]")
{
    REQUIRE(CustomFields::choices("a|Alpha,b|Beta") == QList<CustomFields::Choice>{
                                                           {"a", "Alpha"},
                                                           {"b", "Beta" }
    });
    REQUIRE(CustomFields::choices("plain,b|Beta") == QList<CustomFields::Choice>{
                                                         {"plain", "plain"},
                                                         {"b",     "Beta" }
    });
    REQUIRE(CustomFields::choices(",a|Alpha") == QList<CustomFields::Choice>{
                                                     {"a", "Alpha"}
    });
    REQUIRE(CustomFields::choices("").isEmpty());
}

TEST_CASE("CustomFields - defaults", "[customFields][unit]")
{
    SECTION("A choice defaults to its first option")
    {
        const auto field = resolveOne(requiredRule("kind", "choice", "a|Alpha,b|Beta"), {});
        REQUIRE(field.value == "a");
        REQUIRE(field.state == CustomFields::State::Ok);
    }

    SECTION("A choice starting with an empty entry has no default and is asked for")
    {
        const auto field = resolveOne(requiredRule("kind", "choice-search", ",a|Alpha,b|Beta"), {});
        REQUIRE(field.value.isEmpty());
        REQUIRE(field.state == CustomFields::State::Missing);
        REQUIRE(field.error == CustomFields::Error::Empty);
    }

    SECTION("Other types default to the raw value")
    {
        REQUIRE(resolveOne(requiredRule("ticket", "text", "N/A"), {}).value == "N/A");
    }

    SECTION("The entry's value wins over the default")
    {
        REQUIRE(resolveOne(requiredRule("kind", "choice", "a|Alpha,b|Beta"), "b").value == "b");
        REQUIRE(resolveOne(requiredRule("ticket", "text", "N/A"), "KEM-12").value == "KEM-12");
    }
}

TEST_CASE("CustomFields - missing values", "[customFields][unit]")
{
    REQUIRE(resolveOne(requiredRule("ticket", "text"), {}).state == CustomFields::State::Missing);
    REQUIRE(resolveOne(requiredRule("ticket", "textarea"), "  ").state == CustomFields::State::Missing);

    SECTION("An unchecked required checkbox is missing, like on the web")
    {
        REQUIRE(resolveOne(requiredRule("billable", "boolean"), "0").state == CustomFields::State::Missing);
        REQUIRE(resolveOne(requiredRule("billable", "boolean"), {}).state == CustomFields::State::Missing);
        REQUIRE(resolveOne(requiredRule("billable", "boolean"), "1").state == CustomFields::State::Ok);
    }
}

TEST_CASE("CustomFields - fields Kemai cannot fill", "[customFields][unit]")
{
    REQUIRE(resolveOne(requiredRule("template", "invoice-template"), {}).state == CustomFields::State::Unsupported);
    REQUIRE(resolveOne(requiredRule("kind", "choice"), {}).state == CustomFields::State::Unsupported);
    REQUIRE(resolveOne(requiredRule("kind", "choice", ","), {}).state == CustomFields::State::Unsupported);
    REQUIRE(resolveOne(requiredRule("future", "some-new-type"), "x").state == CustomFields::State::Unsupported);
}

TEST_CASE("CustomFields - values checked by type", "[customFields][unit]")
{
    using Error      = CustomFields::Error;
    const auto check = [](const QString &type, const QString &value, const QString &rawValue = {})
    {
        const auto field = resolveOne(requiredRule("field", type, rawValue), value);
        REQUIRE(field.state == (field.error == Error::None ? CustomFields::State::Ok : CustomFields::State::Invalid));
        return field.error;
    };

    SECTION("Text")
    {
        REQUIRE(check("text", "anything") == Error::None);
        REQUIRE(check("textarea", "line 1\nline 2") == Error::None);
    }

    SECTION("Whole number")
    {
        REQUIRE(check("integer", "42") == Error::None);
        REQUIRE(check("integer", "-3") == Error::None);
        REQUIRE(check("integer", "4.2") == Error::NotWholeNumber);
        REQUIRE(check("integer", "abc") == Error::NotWholeNumber);
    }

    SECTION("Number")
    {
        REQUIRE(check("number", "4.25") == Error::None);
        REQUIRE(check("number", "-4") == Error::None);
        REQUIRE(check("number", "4,25") == Error::NotNumber);
        REQUIRE(check("number", "4.") == Error::NotNumber);
    }

    SECTION("Money: a plain decimal")
    {
        REQUIRE(check("money", "123") == Error::None);
        REQUIRE(check("money", "123.45") == Error::None);
        REQUIRE(check("money", "123,45") == Error::NotMoney);
        REQUIRE(check("money", "12 €") == Error::NotMoney);
    }

    SECTION("Duration: whole seconds")
    {
        REQUIRE(check("duration", "1242000") == Error::None);
        REQUIRE(check("duration", "345:00") == Error::NotDuration);
        REQUIRE(check("duration", "1.5") == Error::NotDuration);
    }

    SECTION("Date and date-time")
    {
        REQUIRE(check("date", "2026-10-01") == Error::None);
        REQUIRE(check("date", "2026-02-30") == Error::NotDate);
        REQUIRE(check("date", "10/1/2026") == Error::NotDate);
        REQUIRE(check("datetime", "2026-10-22 08:30:00") == Error::None);
        REQUIRE(check("datetime", "2026-10-22T08:30:00") == Error::NotDateTime);
        REQUIRE(check("datetime", "2026-10-22") == Error::NotDateTime);
    }

    SECTION("Email")
    {
        REQUIRE(check("email", "john@example.com") == Error::None);
        REQUIRE(check("email", "john@") == Error::NotEmail);
        REQUIRE(check("email", "john@example") == Error::NotEmail);
        REQUIRE(check("email", "john doe@example.com") == Error::NotEmail);
    }

    SECTION("URL")
    {
        REQUIRE(check("url", "https://www.kimai.org/documentation/") == Error::None);
        REQUIRE(check("url", "http://localhost:8001") == Error::None);
        REQUIRE(check("url", "www.kimai.org") == Error::NotUrl);
        REQUIRE(check("url", "ftp://files.example.com") == Error::NotUrl);
    }

    SECTION("Color")
    {
        REQUIRE(check("color", "#5319e7") == Error::None);
        REQUIRE(check("color-free", "#ABCDEF") == Error::None);
        REQUIRE(check("color-free", "red") == Error::NotColor);
        REQUIRE(check("color-free", "#abc") == Error::NotColor);
    }

    SECTION("Choice")
    {
        REQUIRE(check("choice", "b", "a|Alpha,b|Beta") == Error::None);
        REQUIRE(check("choice-search", "Beta", "a|Alpha,b|Beta") == Error::NotInOptions);
    }

    SECTION("Country, currency, language")
    {
        REQUIRE(check("country", "FR") == Error::None);
        REQUIRE(check("country", "XX") == Error::UnknownCountry);
        REQUIRE(check("currency", "EUR") == Error::None);
        REQUIRE(check("currency", "eur") == Error::UnknownCurrency);
        REQUIRE(check("language", "pt_BR") == Error::None);
        REQUIRE(check("language", "klingon") == Error::UnknownLanguage);
    }
}

// Replies recorded on demo.kimai.org (MetaFieldsBundle), entry 11517 saved in the web form with every field filled
TEST_CASE("CustomFields - recorded definitions and values", "[customFields][unit]")
{
    const auto read = [](const QString &name)
    {
        QFile file(QStringLiteral(KEMAI_TEST_DATA_DIR "/kimai-metafields/%1.json").arg(name));
        REQUIRE(file.open(QIODevice::ReadOnly));
        return QJsonDocument::fromJson(file.readAll());
    };

    KimaiMetaFieldRules rules;
    for (const auto &value : read("metafields-timesheet").array())
    {
        rules.append(KimaiMetaFieldRule::fromJson(value));
    }
    REQUIRE(rules.size() == 18);

    const auto entry = KimaiTimeSheet::fromJson(read("timesheet-values").object());
    REQUIRE(entry.project.id == 35);

    const CustomFields::KnownCodes codes{.countries = {"AF"}, .currencies = {"AFN"}, .languages = {"ab"}};
    const auto                     fields = CustomFields::resolve(rules, 3, entry.project.id, entry.activity.id, entry.metaFields, codes);
    REQUIRE(fields.size() == 18);

    for (const auto &field : fields)
    {
        INFO(field.rule.name.toStdString() << " = " << field.value.toStdString());
        if (field.rule.name == "kemai_test_project_customer")
        {
            // Restricted to the entry's customer and project, added after the entry was saved
            REQUIRE(field.state == CustomFields::State::Missing);
        }
        else
        {
            REQUIRE(field.state == CustomFields::State::Ok);
        }
    }

    SECTION("Choice defaults")
    {
        const auto rule = [&rules](const QString &name)
        { return *std::ranges::find(rules, name, &KimaiMetaFieldRule::name); };
        REQUIRE(CustomFields::defaultValue(rule("kemai_test_dropdown")) == "a");
        REQUIRE(CustomFields::defaultValue(rule("kemai_test_choice")).isEmpty());
        REQUIRE(CustomFields::choices(rule("kemai_test_choice").rawValue) == QList<CustomFields::Choice>{
                                                                                 {"a", "alpha"},
                                                                                 {"b", "beta" }
        });
    }
}
