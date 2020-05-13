#pragma once

//#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>

#include "kemai/kemaiupdater.h"

namespace kemai::updater {

class KemaiUpdater::KemaiUpdaterPrivate : public QObject
{
Q_OBJECT

public:
    explicit KemaiUpdaterPrivate(KemaiUpdater* c);

    QNetworkRequest prepareGithubRequest(const QString& path);

public:
    QVersionNumber currentChecksinceVersion;
    bool silenceIfNoNew;

    QScopedPointer<QNetworkAccessManager> networkAccessManager;

private slots:
    void onNamFinished(QNetworkReply* reply);

private:
    KemaiUpdater* const mQ;
};

} // namespace kemai::client
