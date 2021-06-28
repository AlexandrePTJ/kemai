#pragma once

#include <optional>

#include <QAbstractListModel>

namespace kemai::qmlapp {

enum KimaiRoles
{
    KimaiIdRole = Qt::UserRole + 1,
    NameRole,
    CustomerIdRole,
    ProjectIdRole,
    ActivityIdRole
};

template<class T> class KimaiBaseDataModel : public QAbstractListModel
{
public:
    void setValues(const QVector<T>& values)
    {
        // Always set an empty value to reset filters
        mValues.clear();

        T empty;
        empty.id = -1;
        mValues << empty;

        mValues << values;
        beginResetModel();
        endResetModel();
        emit dataChanged(index(0, 0), index(mValues.size(), 0));
    }

    int rowCount(const QModelIndex& parent) const override { return mValues.size(); }

    QVariant data(const QModelIndex& index, int role) const override
    {
        int i = index.row();
        if (i < 0 || i >= mValues.size())
        {
            return QVariant();
        }

        const auto& value = mValues.at(i);

        switch (role)
        {
        /*
         * Generic roles
         */
        case NameRole:
            return value.name;

        case KimaiIdRole:
            return value.id;

        /*
         * Specialized roles
         */
        case CustomerIdRole: {
            auto opt_id = customerId(value);
            if (opt_id)
                return opt_id.value();
        }
        break;

        case ProjectIdRole: {
            auto opt_id = projectId(value);
            if (opt_id)
                return opt_id.value();
        }
        break;

        case ActivityIdRole: {
            auto opt_id = activityId(value);
            if (opt_id)
                return opt_id.value();
        }
        break;

        default:
            break;
        }

        return QVariant();
    }

    std::optional<int> customerId(const T& t) const;
    std::optional<int> projectId(const T& t) const;
    std::optional<int> activityId(const T& t) const;

    QHash<int, QByteArray> roleNames() const override
    {
        QHash<int, QByteArray> roles;
        roles[KimaiIdRole]    = "kimaiId";
        roles[NameRole]       = "name";
        roles[CustomerIdRole] = "customerId";
        roles[ProjectIdRole]  = "projectId";
        roles[ActivityIdRole] = "activityId";
        return roles;
    }

private:
    QVector<T> mValues;
};

} // namespace kemai::qmlapp
