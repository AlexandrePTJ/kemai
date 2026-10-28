// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// Qt headers
#include <QObject>
#include <QVariantList>
#include <QtQmlIntegration/qqmlintegration.h>

// Kemai headers
#include <misc/customFields.h>

namespace kemai
{
    /*!
     * Lists Kimai asks for when creating a customer (ISO countries and currencies, IANA timezones), and the
     * system's own values to prefill them. Read once, shared by every instance. The country, currency and language
     * lists are those Kimai's forms offer (custom fields included).
     */
    class ReferenceData : public QObject
    {
        Q_OBJECT
        QML_ELEMENT

    public:
        explicit ReferenceData(QObject *parent = nullptr);

        // {code, name}, sorted by name
        Q_INVOKABLE QVariantList countries() const;
        // {code, name}, current currencies only, sorted by name
        Q_INVOKABLE QVariantList currencies() const;
        // {code, name}, sorted by name
        Q_INVOKABLE QVariantList languages() const;
        // IANA ids, sorted
        Q_INVOKABLE QStringList timezones() const;

        // ISO codes of the system locale, empty when unknown or not in the lists
        Q_INVOKABLE QString systemCountry() const;
        Q_INVOKABLE QString systemCurrency() const;

        // Every code a country, currency (historic ones included) or language custom field may hold
        static CustomFields::KnownCodes knownCodes();
    };
} // namespace kemai
