// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// Qt headers
#include <QDateTime>
#include <QPointer>
#include <QVariantMap>
#include <QtQmlIntegration/qqmlintegration.h>

// Project headers
#include "sessionContext.h"
#include <model/customFieldsModel.h>

namespace kemai
{
    /*!
     * The custom fields section of the time entry dialog: CustomFieldsModel fed from the session (rules, palette,
     * known codes, the project's customer and currency, the user's time zone for date-times).
     */
    class CustomFieldsForm : public CustomFieldsModel
    {
        Q_OBJECT
        QML_ELEMENT

        Q_PROPERTY(kemai::SessionContext *sessionContext READ sessionContext WRITE setSessionContext NOTIFY sessionContextChanged)

    public:
        explicit CustomFieldsForm(QObject *parent = nullptr);

        SessionContext *sessionContext() const;
        void            setSessionContext(SessionContext *sessionContext);

        // Starts over on an entry; `values` are its custom values (TimesheetModel customValues role), empty for a new one
        Q_INVOKABLE void load(int projectId, int activityId, const QVariantMap &values);
        Q_INVOKABLE void moveTo(int projectId, int activityId);
        // Drafts to save (name -> stored value)
        Q_INVOKABLE QVariantMap valueMap() const;

        // Date and date-time fields, in local time; invalid when the field holds none
        Q_INVOKABLE QDateTime dateTimeAt(int row) const;
        Q_INVOKABLE void      setDateTime(int row, const QDateTime &dateTime);
        Q_INVOKABLE void      setDate(int row, const QDateTime &date);

    signals:
        void sessionContextChanged();

    private:
        QTimeZone userZone() const;

        QPointer<SessionContext> m_sessionContext;
    };
} // namespace kemai
