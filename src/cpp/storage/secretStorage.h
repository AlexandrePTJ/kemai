#pragma once

// Qt headers
#include <QObject>
#include <QString>

namespace kemai
{
    class SecretStorage : public QObject
    {
        Q_OBJECT

    public:
        explicit SecretStorage(QObject *parent = nullptr);
        ~SecretStorage() override;

        void readSecret(const QString &key);
        void writeSecret(const QString &key, const QString &value);
        void deleteSecret(const QString &key);

    signals:
        void secretRead(const QString &key, const QString &value);
        void secretWritten(const QString &key);
        void secretDeleted(const QString &key);
        void readErrorOccurred(const QString &key, const QString &message);
        void writeErrorOccurred(const QString &key, const QString &message);
        void deleteErrorOccurred(const QString &key, const QString &message);
    };

} // namespace kemai