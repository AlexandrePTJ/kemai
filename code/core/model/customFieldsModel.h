// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// Qt headers
#include <QAbstractListModel>
#include <QLocale>
#include <QSet>
#include <QVariantList>

// Project headers
#include <misc/customFields.h>

namespace kemai
{
    /*!
     * The required custom fields of one time entry being edited or started: one row per field that applies, in
     * Kimai's order, with the draft value and its state. Values are kept in the format Kimai stores; typed text goes
     * through setInput, picked values through setValue. Moving the entry to another project or activity keeps the
     * values of the fields that still apply and marks the new ones.
     */
    class CustomFieldsModel : public QAbstractListModel
    {
        Q_OBJECT

        Q_PROPERTY(int count READ rowCount NOTIFY fieldsChanged)
        // Supported fields still empty / holding a value that does not fit
        Q_PROPERTY(int missingCount READ missingCount NOTIFY statesChanged)
        Q_PROPERTY(int invalidCount READ invalidCount NOTIFY statesChanged)
        // Nothing missing or invalid: the entry may be started or saved
        Q_PROPERTY(bool valid READ isValid NOTIFY statesChanged)
        // Currency code of the entry's customer (suffix of money fields), empty when unknown
        Q_PROPERTY(QString currency READ currency NOTIFY fieldsChanged)
        // The instance's palette for color fields: {name, color}
        Q_PROPERTY(QVariantList colorChoices READ colorChoices NOTIFY fieldsChanged)
        // Kimai types of the rows (which reference lists the inputs need)
        Q_PROPERTY(QStringList types READ types NOTIFY fieldsChanged)

    public:
        enum class Role
        {
            Name = Qt::UserRole + 1,
            Label,     // Kimai's label, else the field name
            Help,      // may be empty
            Type,      // Kimai's type name
            Value,     // draft, in the stored format
            InputText, // the value as the user types it (durations h:mm, the locale's decimal separator)
            State,     // "ok" | "missing" | "invalid" | "unsupported"
            Error,     // CustomFields::Error name ("None", "Empty", "NotEmail"...)
            Choices,   // choice fields: [{key, label}]
            Appeared,  // the field applies since the last project/activity change
            Unsaved    // a write of this value failed (partial save)
        };

        explicit CustomFieldsModel(QObject *parent = nullptr);

        void setRules(const KimaiMetaFieldRules &rules);
        void setKnownCodes(const CustomFields::KnownCodes &codes);
        void setColorChoices(const KimaiColorChoices &colors);
        // Locale of the typed numbers (the system's by default)
        void setLocale(const QLocale &locale);

        // Starts over on an entry: `values` are the entry's (name -> stored value), defaults fill the empty ones
        void load(int customerId, int projectId, int activityId, const QMap<QString, QString> &values, const QString &currency);
        // The entry moved: fields that still apply keep their draft, new ones get their default and are marked
        void moveTo(int customerId, int projectId, int activityId, const QString &currency);

        Q_INVOKABLE void setInput(int row, const QString &text);
        Q_INVOKABLE void setValue(int row, const QString &stored);
        // Marks the fields whose write failed; cleared by load
        Q_INVOKABLE void markUnsaved(const QStringList &names);
        // First row missing or invalid, -1 when none
        Q_INVOKABLE int firstIncompleteRow() const;

        // Drafts of the fields Kemai can fill (name -> stored value)
        QMap<QString, QString> values() const;

        int          missingCount() const;
        int          invalidCount() const;
        bool         isValid() const;
        QString      currency() const;
        QVariantList colorChoices() const;
        QStringList  types() const;

        int                    rowCount(const QModelIndex &parent = {}) const override;
        QVariant               data(const QModelIndex &index, int role) const override;
        QHash<int, QByteArray> roleNames() const override;

    signals:
        void fieldsChanged();
        void statesChanged();

    private:
        struct Row
        {
            CustomFields::Field field;
            CustomFields::Error inputError = CustomFields::Error::None; // typed text that does not parse
            bool                appeared   = false;
        };

        void                       resolveRows(int customerId, int projectId, int activityId, const QSet<QString> &previous);
        void                       updateRow(int row, const QString &stored, CustomFields::Error inputError);
        static CustomFields::State stateOf(const Row &row);

        KimaiMetaFieldRules      m_rules;
        CustomFields::KnownCodes m_codes;
        KimaiColorChoices        m_colors;
        QLocale                  m_locale;
        QList<Row>               m_rows;
        QMap<QString, QString>   m_drafts;
        QSet<QString>            m_unsaved;
        QString                  m_currency;
    };
} // namespace kemai
