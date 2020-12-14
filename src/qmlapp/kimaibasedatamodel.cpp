#include "kimaibasedatamodel.h"

using namespace kemai::qmlapp;
//
// template<class T> int KimaiBaseDataModel<T>::rowCount(const QModelIndex& parent) const
//{
//    return mValues.size();
//}
//
// template<class T> QVariant KimaiBaseDataModel<T>::data(const QModelIndex& index, int role) const
//{
//    int i = index.row();
//    if (i < 0 || i >= mValues.size())
//    {
//        return QVariant(QVariant::Invalid);
//    }
//
//    const auto& value = mValues.at(i);
//    switch (role)
//    {
//    case Qt::DisplayRole:
//        return value.name;
//
//    case KimaiIdRole:
//        return value.id;
//
//    default:
//        break;
//    }
//
//    return QVariant();
//}
//
// template<class T> QHash<int, QByteArray> KimaiBaseDataModel<T>::roleNames() const
//{
//    QHash<int, QByteArray> roles;
//    roles[KimaiIdRole] = "kimaiId";
//    return roles;
//}