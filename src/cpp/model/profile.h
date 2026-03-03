#pragma once

// STL headers
#include <vector>

// Qt headers
#include <QAbstractListModel>

// Project headers
#include <data/profile.h>

namespace kemai
{
    class ProfileModel : public QAbstractListModel
    {
        Q_OBJECT

    public:
        enum class ProfileRole
        {
            ProfileId = Qt::UserRole + 1,
            Name,
            Host,
            Token
        };

        explicit ProfileModel(QObject *parent = nullptr);
        ~ProfileModel() override;

        void setProfiles(const std::vector<Profile> &profiles);

        Q_INVOKABLE QVariantMap getByIndex(int index) const;
        Q_INVOKABLE QVariantMap getById(const ProfileId &id) const;

        int                    rowCount(const QModelIndex &parent) const override;
        QVariant               data(const QModelIndex &index, int role) const override;
        QHash<int, QByteArray> roleNames() const override;

    private:
        std::vector<Profile> m_profiles;
    };

} // namespace kemai
