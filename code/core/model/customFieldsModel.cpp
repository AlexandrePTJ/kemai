// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "customFieldsModel.h"

// STL headers
#include <algorithm>

// magic_enum headers
#include <magic_enum/magic_enum.hpp>

namespace kemai
{
    CustomFieldsModel::CustomFieldsModel(QObject *parent):
    QAbstractListModel(parent)
    {
    }

    void CustomFieldsModel::setRules(const KimaiMetaFieldRules &rules)
    {
        m_rules = rules;
    }

    void CustomFieldsModel::setKnownCodes(const CustomFields::KnownCodes &codes)
    {
        m_codes = codes;
    }

    void CustomFieldsModel::setColorChoices(const KimaiColorChoices &colors)
    {
        m_colors = colors;
    }

    void CustomFieldsModel::setLocale(const QLocale &locale)
    {
        m_locale = locale;
    }

    void CustomFieldsModel::load(int customerId, int projectId, int activityId, const QMap<QString, QString> &values, const QString &currency)
    {
        m_drafts = values;
        m_unsaved.clear();
        m_currency = currency;
        resolveRows(customerId, projectId, activityId, {});
    }

    void CustomFieldsModel::moveTo(int customerId, int projectId, int activityId, const QString &currency)
    {
        QSet<QString> previous;
        for (const auto &row : std::as_const(m_rows))
        {
            previous.insert(row.field.rule.name);
        }
        m_currency = currency;
        resolveRows(customerId, projectId, activityId, previous);
    }

    void CustomFieldsModel::resolveRows(int customerId, int projectId, int activityId, const QSet<QString> &previous)
    {
        beginResetModel();
        m_rows.clear();
        for (const auto &field : CustomFields::resolve(m_rules, customerId, projectId, activityId, m_drafts, m_codes))
        {
            const auto &name = field.rule.name;
            m_drafts.insert(name, field.value);

            Row row{.field = field, .appeared = !previous.isEmpty() && !previous.contains(name)};
            // Text typed earlier that did not parse is still what the field holds
            if (const auto input = CustomFields::parseInput(field.rule.type, field.value, m_locale); input.error != CustomFields::Error::None)
            {
                row.inputError = input.error;
            }
            m_rows.append(row);
        }
        endResetModel();
        emit fieldsChanged();
        emit statesChanged();
    }

    void CustomFieldsModel::setInput(int row, const QString &text)
    {
        if (row < 0 || row >= m_rows.size())
        {
            return;
        }
        const auto input = CustomFields::parseInput(m_rows[row].field.rule.type, text, m_locale);
        updateRow(row, input.stored, input.error);
    }

    void CustomFieldsModel::setValue(int row, const QString &stored)
    {
        if (row < 0 || row >= m_rows.size())
        {
            return;
        }
        updateRow(row, stored, CustomFields::Error::None);
    }

    void CustomFieldsModel::updateRow(int row, const QString &stored, CustomFields::Error inputError)
    {
        auto &entry = m_rows[row];
        if (entry.field.value == stored && entry.inputError == inputError)
        {
            return;
        }
        m_drafts.insert(entry.field.rule.name, stored);
        entry.field      = CustomFields::check(entry.field.rule, stored, m_codes);
        entry.inputError = inputError;

        // InputText is left out: the field being typed in keeps its own text
        emit dataChanged(index(row), index(row), {static_cast<int>(Role::Value), static_cast<int>(Role::State), static_cast<int>(Role::Error)});
        emit statesChanged();
    }

    void CustomFieldsModel::markUnsaved(const QStringList &names)
    {
        m_unsaved = QSet<QString>(names.cbegin(), names.cend());
        if (!m_rows.isEmpty())
        {
            emit dataChanged(index(0), index(static_cast<int>(m_rows.size()) - 1), {static_cast<int>(Role::Unsaved)});
        }
    }

    int CustomFieldsModel::firstIncompleteRow() const
    {
        const auto it = std::ranges::find_if(m_rows, [](const Row &row)
                                             { const auto state = stateOf(row); return state == CustomFields::State::Missing || state == CustomFields::State::Invalid; });
        return it == m_rows.cend() ? -1 : static_cast<int>(std::distance(m_rows.cbegin(), it));
    }

    QMap<QString, QString> CustomFieldsModel::values() const
    {
        QMap<QString, QString> result;
        for (const auto &row : m_rows)
        {
            if (row.field.state != CustomFields::State::Unsupported)
            {
                result.insert(row.field.rule.name, row.field.value);
            }
        }
        return result;
    }

    CustomFields::State CustomFieldsModel::stateOf(const Row &row)
    {
        if (row.field.state == CustomFields::State::Unsupported || row.field.state == CustomFields::State::Missing)
        {
            return row.field.state;
        }
        return row.inputError != CustomFields::Error::None ? CustomFields::State::Invalid : row.field.state;
    }

    int CustomFieldsModel::missingCount() const
    {
        return static_cast<int>(std::ranges::count_if(m_rows, [](const Row &row)
                                                      { return stateOf(row) == CustomFields::State::Missing; }));
    }

    int CustomFieldsModel::invalidCount() const
    {
        return static_cast<int>(std::ranges::count_if(m_rows, [](const Row &row)
                                                      { return stateOf(row) == CustomFields::State::Invalid; }));
    }

    bool CustomFieldsModel::isValid() const
    {
        return missingCount() == 0 && invalidCount() == 0;
    }

    QString CustomFieldsModel::currency() const
    {
        return m_currency;
    }

    QVariantList CustomFieldsModel::colorChoices() const
    {
        QVariantList result;
        for (const auto &color : m_colors)
        {
            result.append(QVariantMap{
                {"name",  color.name },
                {"color", color.color}
            });
        }
        return result;
    }

    QStringList CustomFieldsModel::types() const
    {
        QStringList result;
        for (const auto &row : m_rows)
        {
            result.append(row.field.rule.type);
        }
        return result;
    }

    int CustomFieldsModel::rowCount(const QModelIndex &parent) const
    {
        return parent.isValid() ? 0 : static_cast<int>(m_rows.size());
    }

    QVariant CustomFieldsModel::data(const QModelIndex &index, int role) const
    {
        if (!index.isValid() || index.row() >= m_rows.size())
        {
            return {};
        }

        const auto &row   = m_rows.at(index.row());
        const auto &rule  = row.field.rule;
        const auto  state = stateOf(row);
        switch (static_cast<Role>(role))
        {
        case Role::Name:
            return rule.name;
        case Role::Label:
            return rule.label.isEmpty() ? rule.name : rule.label;
        case Role::Help:
            return rule.help;
        case Role::Type:
            return rule.type;
        case Role::Value:
            return row.field.value;
        case Role::InputText:
            return row.inputError != CustomFields::Error::None ? row.field.value : CustomFields::inputText(rule.type, row.field.value, m_locale);
        case Role::State:
            return QString::fromUtf8(magic_enum::enum_name(state).data()).toLower();
        case Role::Error:
        {
            const auto error = row.inputError != CustomFields::Error::None && state == CustomFields::State::Invalid ? row.inputError : row.field.error;
            return QString::fromUtf8(magic_enum::enum_name(error).data());
        }
        case Role::Choices:
        {
            QVariantList choices;
            for (const auto &choice : CustomFields::choices(rule.rawValue))
            {
                choices.append(QVariantMap{
                    {"key",   choice.value},
                    {"label", choice.label}
                });
            }
            return choices;
        }
        case Role::Appeared:
            return row.appeared;
        case Role::Unsaved:
            return m_unsaved.contains(rule.name);
        }
        return {};
    }

    QHash<int, QByteArray> CustomFieldsModel::roleNames() const
    {
        return {
            {static_cast<int>(Role::Name),      "name"     },
            {static_cast<int>(Role::Label),     "label"    },
            {static_cast<int>(Role::Help),      "help"     },
            {static_cast<int>(Role::Type),      "type"     },
            {static_cast<int>(Role::Value),     "value"    },
            {static_cast<int>(Role::InputText), "inputText"},
            {static_cast<int>(Role::State),     "state"    },
            {static_cast<int>(Role::Error),     "error"    },
            {static_cast<int>(Role::Choices),   "choices"  },
            {static_cast<int>(Role::Appeared),  "appeared" },
            {static_cast<int>(Role::Unsaved),   "unsaved"  }
        };
    }
} // namespace kemai
