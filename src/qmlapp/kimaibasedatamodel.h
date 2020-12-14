#pragma once

#include <QAbstractListModel>

namespace kemai::qmlapp {

template<class T> class KimaiBaseDataModel : public QAbstractListModel
{
public:
    enum CustomerRoles
    {
        KimaiIdRole = Qt::UserRole + 1,
        NameRole
    };

    void setValues(const QVector<T>& values)
    {
        mValues = values;
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
            return QVariant(QVariant::Invalid);
        }

        const auto& value = mValues.at(i);
        switch (role)
        {
        case NameRole:
            return value.name;

        case KimaiIdRole:
            return value.id;

        default:
            break;
        }

        return QVariant();
    }

    QHash<int, QByteArray> roleNames() const override
    {
        QHash<int, QByteArray> roles;
        roles[KimaiIdRole] = "kimaiId";
        roles[NameRole]    = "name";
        return roles;
    }

private:
    QVector<T> mValues;
};

} // namespace kemai::qmlapp
