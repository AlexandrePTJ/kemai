// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "referenceData.h"

// STL headers
#include <algorithm>

// Qt headers
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QTimeZone>

namespace
{
    // {code, name} from a {"CODE": "Name"} resource, sorted by name; `keep` filters by name
    template<typename Keep>
    QVariantList readCodeNames(const QString &resource, Keep keep)
    {
        QFile file(resource);
        if (!file.open(QIODevice::ReadOnly))
        {
            return {};
        }

        const auto   object = QJsonDocument::fromJson(file.readAll()).object();
        QVariantList list;
        for (auto it = object.begin(); it != object.end(); ++it)
        {
            const auto name = it.value().toString();
            if (keep(name))
            {
                list.append(QVariantMap{{"code", it.key()}, {"name", name}});
            }
        }
        std::ranges::sort(list, [](const QVariant &a, const QVariant &b)
                          { return a.toMap().value("name").toString().localeAwareCompare(b.toMap().value("name").toString()) < 0; });
        return list;
    }

    bool containsCode(const QVariantList &list, const QString &code)
    {
        return std::ranges::any_of(list, [&code](const QVariant &item) { return item.toMap().value("code").toString() == code; });
    }
} // namespace

namespace kemai
{
    ReferenceData::ReferenceData(QObject *parent): QObject(parent) {}

    QVariantList ReferenceData::countries() const
    {
        static const auto list = readCodeNames(":/assets/data/countries.json", [](const QString &) { return true; });
        return list;
    }

    QVariantList ReferenceData::currencies() const
    {
        // Historic currencies carry their years, e.g. "Afghan Afghani (1927–2002)": Kimai offers only current ones
        static const auto list = readCodeNames(":/assets/data/currencies.json", [](const QString &name) { return !name.contains('('); });
        return list;
    }

    QStringList ReferenceData::timezones() const
    {
        static const auto list = []
        {
            QStringList ids;
            for (const auto &id : QTimeZone::availableTimeZoneIds())
            {
                // IANA "Area/Location" names only, as Kimai lists them (no "UTC+01:00" style ids)
                if (id.contains('/') && !id.startsWith("Etc/"))
                {
                    ids.append(QString::fromLatin1(id));
                }
            }
            ids.append(QStringLiteral("UTC"));
            ids.sort();
            return ids;
        }();
        return list;
    }

    QString ReferenceData::systemCountry() const
    {
        const auto code = QLocale::territoryToCode(QLocale::system().territory());
        return containsCode(countries(), code) ? code : QString();
    }

    QString ReferenceData::systemCurrency() const
    {
        const auto code = QLocale::system().currencySymbol(QLocale::CurrencyIsoCode);
        return containsCode(currencies(), code) ? code : QString();
    }
} // namespace kemai
