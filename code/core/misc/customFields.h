// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// Qt headers
#include <QDateTime>
#include <QList>
#include <QLocale>
#include <QMap>
#include <QSet>
#include <QString>
#include <QTimeZone>

// Kemai headers
#include "client/kimaiTimesheets.h"

namespace kemai
{
    /*!
     * Decides which required custom fields (MetaFieldsBundle) apply to a time entry, with which value, and whether
     * that value fits the field's type. Pure: the caller gives the rules, the entry's resources and values, and the
     * codes it knows. Errors are codes, worded by the UI.
     */
    class CustomFields
    {
    public:
        enum class State
        {
            Ok,
            Missing,    // empty, or an unchecked yes/no field
            Invalid,    // does not fit the type (see error)
            Unsupported // Kemai cannot edit it (invoice template, choice without options, unknown type)
        };

        enum class Error
        {
            None,
            Empty,
            NotWholeNumber,
            NotNumber,
            NotMoney,
            NotDuration,
            MinutesOutOfRange, // a typed h:mm with 60 minutes or more
            NotDate,
            NotDateTime,
            NotEmail,
            NotUrl,
            NotColor,
            NotInOptions,
            UnknownCountry,
            UnknownCurrency,
            UnknownLanguage
        };

        struct Choice
        {
            QString value;
            QString label;

            bool operator==(const Choice &) const = default;
        };

        // Codes a country, currency or language field may hold
        struct KnownCodes
        {
            QSet<QString> countries;
            QSet<QString> currencies;
            QSet<QString> languages;
        };

        struct Field
        {
            KimaiMetaFieldRule rule;
            QString            value;
            State              state = State::Ok;
            Error              error = Error::None;
        };

        // A rule applies when every restriction it sets matches the entry, as on the web (checked on demo.kimai.org)
        static bool applies(const KimaiMetaFieldRule &rule, int customerId, int projectId, int activityId);

        // Options of a choice field: comma-separated `value|label` entries (label = value without `|`), empty
        // entries left out
        static QList<Choice> choices(const QString &rawValue);

        // Value given to a field that has none: for a choice, its first option unless the list starts with an empty
        // entry; otherwise the rule's raw value
        static QString defaultValue(const KimaiMetaFieldRule &rule);

        static bool isSupported(const KimaiMetaFieldRule &rule);

        // Checks a non-empty value against a field's type; Error::None when it fits
        static Error validate(const KimaiMetaFieldRule &rule, const QString &value, const KnownCodes &codes);

        // State of a field holding `value` (no default applied)
        static Field check(const KimaiMetaFieldRule &rule, const QString &value, const KnownCodes &codes);

        // What a typed value is stored as: durations typed `h:mm`, `h` or decimal hours become whole seconds, numbers
        // and amounts get a dot as decimal separator. A text that does not parse comes back as typed, with its error.
        struct Input
        {
            QString stored;
            Error   error = Error::None;
        };
        static Input parseInput(const QString &type, const QString &text, const QLocale &locale);
        // A stored value as the user types it (inverse of parseInput); other types unchanged
        static QString inputText(const QString &type, const QString &stored, const QLocale &locale);

        // Date-times are stored in the Kimai user's time zone, without offset
        static QString   storedDateTime(const QDateTime &dateTime, const QTimeZone &userZone);
        static QDateTime dateTimeFromStored(const QString &stored, const QTimeZone &userZone);

        // The required fields that apply to the entry, in the rules' order, each with its value (the entry's, else the
        // default) and its state
        static QList<Field> resolve(const KimaiMetaFieldRules &rules, int customerId, int projectId, int activityId, const QMap<QString, QString> &values, const KnownCodes &codes);
    };
} // namespace kemai
