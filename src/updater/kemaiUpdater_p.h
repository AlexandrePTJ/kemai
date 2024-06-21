#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>

#include "kemaiUpdater.h"

namespace kemai {

class KemaiUpdater::KemaiUpdaterPrivate : public QObject
{
    Q_OBJECT

public:
    explicit KemaiUpdaterPrivate(KemaiUpdater* c);

    QNetworkRequest prepareGithubRequest(const QString& path);
    void onLatestReleaseReplyFinished();

    QVersionNumber currentCheckSinceVersion;
    bool silenceIfNoNew;

    std::shared_ptr<QNetworkAccessManager> networkAccessManager;

private:
    KemaiUpdater* const mQ;
};

} // namespace kemai
