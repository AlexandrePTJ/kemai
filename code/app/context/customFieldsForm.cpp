// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "customFieldsForm.h"

namespace kemai
{
    CustomFieldsForm::CustomFieldsForm(QObject *parent):
    CustomFieldsModel(parent)
    {
    }

    SessionContext *CustomFieldsForm::sessionContext() const
    {
        return m_sessionContext;
    }

    void CustomFieldsForm::setSessionContext(SessionContext *sessionContext)
    {
        if (m_sessionContext == sessionContext)
        {
            return;
        }
        m_sessionContext = sessionContext;
        emit sessionContextChanged();
    }

    void CustomFieldsForm::load(int projectId, int activityId, const QVariantMap &values)
    {
        if (!m_sessionContext)
        {
            return;
        }
        // The definitions may have changed since the last opening (cache refresh)
        setRules(m_sessionContext->customFieldRules());
        setColorChoices(m_sessionContext->colorChoices());
        setKnownCodes(m_sessionContext->knownCodes());

        QMap<QString, QString> stored;
        for (auto it = values.cbegin(); it != values.cend(); ++it)
        {
            stored.insert(it.key(), it.value().toString());
        }
        const auto customer = m_sessionContext->customerOfProject(projectId);
        CustomFieldsModel::load(customer.id, projectId, activityId, stored, customer.currencyKey);
    }

    void CustomFieldsForm::moveTo(int projectId, int activityId)
    {
        if (!m_sessionContext)
        {
            return;
        }
        const auto customer = m_sessionContext->customerOfProject(projectId);
        CustomFieldsModel::moveTo(customer.id, projectId, activityId, customer.currencyKey);
    }

    QVariantMap CustomFieldsForm::valueMap() const
    {
        QVariantMap result;
        const auto  drafts = values();
        for (auto it = drafts.cbegin(); it != drafts.cend(); ++it)
        {
            result.insert(it.key(), it.value());
        }
        return result;
    }

    QDateTime CustomFieldsForm::dateTimeAt(int row) const
    {
        const auto i      = index(row);
        const auto stored = data(i, static_cast<int>(Role::Value)).toString();
        if (data(i, static_cast<int>(Role::Type)).toString() == "date")
        {
            return QDate::fromString(stored, "yyyy-MM-dd").startOfDay();
        }
        return CustomFields::dateTimeFromStored(stored, userZone());
    }

    void CustomFieldsForm::setDateTime(int row, const QDateTime &dateTime)
    {
        setValue(row, CustomFields::storedDateTime(dateTime, userZone()));
    }

    void CustomFieldsForm::setDate(int row, const QDateTime &date)
    {
        setValue(row, date.date().toString("yyyy-MM-dd"));
    }

    QTimeZone CustomFieldsForm::userZone() const
    {
        // Kimai stores date-times in the user's zone; without it, this computer's
        const QTimeZone zone(m_sessionContext ? m_sessionContext->userTimezone().toUtf8() : QByteArray());
        return zone.isValid() ? zone : QTimeZone::systemTimeZone();
    }
} // namespace kemai
