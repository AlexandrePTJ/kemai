// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "customFields.h"

// STL headers
#include <algorithm>

// Qt headers
#include <QDateTime>
#include <QRegularExpression>
#include <QUrl>

namespace
{
    bool isChoice(const QString &type)
    {
        return type == "choice" || type == "choice-search";
    }

    bool matches(const QString &pattern, const QString &value)
    {
        const QRegularExpression expression(QRegularExpression::anchoredPattern(pattern));
        return expression.match(value).hasMatch();
    }

    bool isEmpty(const QString &type, const QString &value)
    {
        // Kimai stores an unchecked yes/no field as "0"; the web form refuses it when required
        return value.trimmed().isEmpty() || (type == "boolean" && value != "1");
    }
} // namespace

namespace kemai
{
    bool CustomFields::applies(const KimaiMetaFieldRule &rule, int customerId, int projectId, int activityId)
    {
        return (rule.customerId == 0 || rule.customerId == customerId) && (rule.projectId == 0 || rule.projectId == projectId) &&
               (rule.activityId == 0 || rule.activityId == activityId);
    }

    QList<CustomFields::Choice> CustomFields::choices(const QString &rawValue)
    {
        QList<Choice> result;
        for (const auto &entry : rawValue.split(','))
        {
            const auto separator = entry.indexOf('|');
            const auto value     = separator < 0 ? entry : entry.left(separator);
            if (value.isEmpty())
            {
                continue;
            }
            result.append({.value = value, .label = separator < 0 ? entry : entry.mid(separator + 1)});
        }
        return result;
    }

    QString CustomFields::defaultValue(const KimaiMetaFieldRule &rule)
    {
        if (!isChoice(rule.type))
        {
            return rule.rawValue;
        }
        if (rule.rawValue.startsWith(','))
        {
            return {};
        }
        const auto options = choices(rule.rawValue);
        return options.isEmpty() ? QString() : options.first().value;
    }

    bool CustomFields::isSupported(const KimaiMetaFieldRule &rule)
    {
        static const QSet<QString> SupportedTypes{"text", "textarea", "integer", "email", "url", "duration", "money", "number", "boolean", "datetime", "date", "color", "color-free", "language", "country", "currency"};
        if (isChoice(rule.type))
        {
            return !choices(rule.rawValue).isEmpty();
        }
        return SupportedTypes.contains(rule.type);
    }

    CustomFields::Error CustomFields::validate(const KimaiMetaFieldRule &rule, const QString &value, const KnownCodes &codes)
    {
        const auto &type = rule.type;
        if (type == "integer")
        {
            return matches(R"(-?\d+)", value) ? Error::None : Error::NotWholeNumber;
        }
        if (type == "number")
        {
            return matches(R"(-?\d+(\.\d+)?)", value) ? Error::None : Error::NotNumber;
        }
        // Stored formats recorded on the demo (code/tests/data/kimai-metafields): money is a plain decimal ("123",
        // shown "123.00"), a duration whole seconds ("1242000" for 345:00), dates ISO, datetimes in the user's time zone
        if (type == "money")
        {
            return matches(R"(-?\d+(\.\d+)?)", value) ? Error::None : Error::NotMoney;
        }
        if (type == "duration")
        {
            return matches(R"(\d+)", value) ? Error::None : Error::NotDuration;
        }
        if (type == "date")
        {
            return QDate::fromString(value, "yyyy-MM-dd").isValid() ? Error::None : Error::NotDate;
        }
        if (type == "datetime")
        {
            return QDateTime::fromString(value, "yyyy-MM-dd HH:mm:ss").isValid() ? Error::None : Error::NotDateTime;
        }
        if (type == "email")
        {
            // Symfony's HTML5 email pattern, which Kimai's form applies
            return matches(R"([a-zA-Z0-9.!#$%&'*+/=?^_`{|}~-]+@[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?(?:\.[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?)+)",
                           value) ?
                       Error::None :
                       Error::NotEmail;
        }
        if (type == "url")
        {
            const QUrl url(value, QUrl::StrictMode);
            return url.isValid() && (url.scheme() == "http" || url.scheme() == "https") && !url.host().isEmpty() ? Error::None : Error::NotUrl;
        }
        if (type == "color" || type == "color-free")
        {
            return matches("#[0-9a-fA-F]{6}", value) ? Error::None : Error::NotColor;
        }
        if (isChoice(type))
        {
            const auto options = choices(rule.rawValue);
            return std::ranges::any_of(options, [&value](const Choice &choice)
                                       { return choice.value == value; }) ?
                       Error::None :
                       Error::NotInOptions;
        }
        if (type == "country")
        {
            return codes.countries.contains(value) ? Error::None : Error::UnknownCountry;
        }
        if (type == "currency")
        {
            return codes.currencies.contains(value) ? Error::None : Error::UnknownCurrency;
        }
        if (type == "language")
        {
            return codes.languages.contains(value) ? Error::None : Error::UnknownLanguage;
        }
        // text, textarea, boolean
        return Error::None;
    }

    CustomFields::Field CustomFields::check(const KimaiMetaFieldRule &rule, const QString &value, const KnownCodes &codes)
    {
        Field field{.rule = rule, .value = value};
        if (!isSupported(rule))
        {
            field.state = State::Unsupported;
        }
        else if (isEmpty(rule.type, value))
        {
            field.state = State::Missing;
            field.error = Error::Empty;
        }
        else
        {
            field.error = validate(rule, value, codes);
            field.state = field.error == Error::None ? State::Ok : State::Invalid;
        }
        return field;
    }

    CustomFields::Input CustomFields::parseInput(const QString &type, const QString &text, const QLocale &locale)
    {
        const auto trimmed = text.trimmed();
        if (trimmed.isEmpty())
        {
            return {};
        }

        if (type == "duration")
        {
            static const QRegularExpression HoursMinutes(QRegularExpression::anchoredPattern(R"((\d+):(\d{1,2}))"));
            static const QRegularExpression DecimalHours(QRegularExpression::anchoredPattern(R"((\d+)(?:[.,](\d+))?)"));
            if (const auto match = HoursMinutes.match(trimmed); match.hasMatch())
            {
                const auto minutes = match.captured(2).toInt();
                if (minutes >= 60)
                {
                    return {.stored = text, .error = Error::MinutesOutOfRange};
                }
                return {.stored = QString::number(match.captured(1).toLongLong() * 3600 + minutes * 60)};
            }
            if (DecimalHours.match(trimmed).hasMatch())
            {
                auto hours = trimmed;
                hours.replace(',', '.');
                return {.stored = QString::number(qRound64(hours.toDouble() * 3600))};
            }
            return {.stored = text, .error = Error::NotDuration};
        }

        if (type == "number" || type == "money")
        {
            // The locale's separator or a dot, as people type either
            auto stored = trimmed;
            stored.replace(locale.decimalPoint(), ".");
            stored.replace(',', '.');
            return {.stored = stored};
        }

        return {.stored = text};
    }

    QString CustomFields::inputText(const QString &type, const QString &stored, const QLocale &locale)
    {
        if (type == "duration" && matches(R"(\d+)", stored))
        {
            const auto minutes = stored.toLongLong() / 60;
            return QStringLiteral("%1:%2").arg(minutes / 60).arg(minutes % 60, 2, 10, QChar('0'));
        }
        if (type == "money" && matches(R"(-?\d+(\.\d+)?)", stored) && !stored.contains('.'))
        {
            // Kimai shows whole amounts with two decimals
            return locale.toString(stored.toDouble(), 'f', 2).remove(locale.groupSeparator());
        }
        if ((type == "number" || type == "money") && matches(R"(-?\d+(\.\d+)?)", stored))
        {
            auto text = stored;
            return text.replace('.', locale.decimalPoint());
        }
        return stored;
    }

    QString CustomFields::storedDateTime(const QDateTime &dateTime, const QTimeZone &userZone)
    {
        return dateTime.toTimeZone(userZone).toString("yyyy-MM-dd HH:mm:ss");
    }

    QDateTime CustomFields::dateTimeFromStored(const QString &stored, const QTimeZone &userZone)
    {
        auto dateTime = QDateTime::fromString(stored, "yyyy-MM-dd HH:mm:ss");
        if (!dateTime.isValid())
        {
            return {};
        }
        dateTime.setTimeZone(userZone, QDateTime::TransitionResolution::RelativeToBefore);
        return dateTime.toLocalTime();
    }

    QList<CustomFields::Field> CustomFields::resolve(const KimaiMetaFieldRules &rules, int customerId, int projectId, int activityId, const QMap<QString, QString> &values, const KnownCodes &codes)
    {
        QList<Field> fields;
        for (const auto &rule : rules)
        {
            if (!rule.required || !applies(rule, customerId, projectId, activityId))
            {
                continue;
            }

            auto value = values.value(rule.name);
            if (const auto fallback = defaultValue(rule); isEmpty(rule.type, value) && !fallback.isEmpty())
            {
                value = fallback;
            }
            fields.append(check(rule, value, codes));
        }
        return fields;
    }
} // namespace kemai
