#include "secretStorage.h"

#include <qtkeychain/keychain.h>

namespace
{
    const auto gServiceName = "kemai";
}

namespace kemai
{
    SecretStorage::SecretStorage(QObject *parent):
    QObject(parent)
    {
    }

    SecretStorage::~SecretStorage() = default;

    void SecretStorage::readSecret(const QString &key)
    {
        auto *job = new QKeychain::ReadPasswordJob(gServiceName, this);
        job->setKey(key);
        job->setAutoDelete(true);

        // clang-format off
        connect(job, &QKeychain::ReadPasswordJob::finished, this, [this, key](QKeychain::Job *j)
                {
                    auto* readJob = qobject_cast<QKeychain::ReadPasswordJob*>(j);
                    if (readJob->error() == QKeychain::NoError)
                    {
                        emit secretRead(key, readJob->textData());
                    }
                    else
                    {
                        emit readErrorOccurred(key, readJob->errorString());
                    }
                }
        );
        // clang-format on

        job->start();
    }

    void SecretStorage::writeSecret(const QString &key, const QString &value)
    {
        auto *job = new QKeychain::WritePasswordJob(gServiceName, this);
        job->setKey(key);
        job->setTextData(value);
        job->setAutoDelete(true);

        // clang-format off
        connect(job, &QKeychain::WritePasswordJob::finished, this, [this, key](QKeychain::Job *j)
                {
                    if (j->error() == QKeychain::NoError)
                    {
                        emit secretWritten(key);
                    }
                    else
                    {
                        emit writeErrorOccurred(key, j->errorString());
                    }
                }
        );
        // clang-format on

        job->start();
    }

    void SecretStorage::deleteSecret(const QString &key)
    {
        auto *job = new QKeychain::DeletePasswordJob(gServiceName, this);
        job->setKey(key);
        job->setAutoDelete(true);

        // clang-format off
        connect(job, &QKeychain::DeletePasswordJob::finished, this, [this, key](QKeychain::Job *j)
                {
                    if (j->error() == QKeychain::NoError)
                    {
                        emit secretDeleted(key);
                    }
                    else
                    {
                        emit deleteErrorOccurred(key, j->errorString());
                    }
                }
        );
        // clang-format on

        job->start();
    }

} // namespace kemai