#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>

#include "kemaiupdater.h"

namespace kemai::updater {

class KemaiUpdater::KemaiUpdaterPrivate : public QObject
{
    Q_OBJECT

public:
    explicit KemaiUpdaterPrivate(KemaiUpdater* c);

    QNetworkRequest prepareGithubRequest(const QString& path);
    void onNamFinished(QNetworkReply* reply);

    QVersionNumber currentCheckSinceVersion;
    bool silenceIfNoNew;

    QScopedPointer<QNetworkAccessManager> networkAccessManager;

private:
    KemaiUpdater* const mQ;
};

} // namespace kemai::updater
