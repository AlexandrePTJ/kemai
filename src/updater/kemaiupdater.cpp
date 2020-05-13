#include "kemai/kemaiupdater.h"
#include "kemaiupdater_p.h"

#include <QJsonObject>
#include <QJsonDocument>

using namespace kemai::updater;

/*
 * Private impl
 */
KemaiUpdater::KemaiUpdaterPrivate::KemaiUpdaterPrivate(KemaiUpdater* c)
    : networkAccessManager(new QNetworkAccessManager), mQ(c)
{
    connect(networkAccessManager.data(), &QNetworkAccessManager::finished, this, &KemaiUpdaterPrivate::onNamFinished);
}

QNetworkRequest KemaiUpdater::KemaiUpdaterPrivate::prepareGithubRequest(const QString& path)
{
    QUrl url;
    url.setScheme("https");
    url.setHost("api.github.com");
    url.setPath(path);

    QNetworkRequest r;
    r.setUrl(url);
    r.setRawHeader("accept", "application/vnd.github.v3+json");
    return r;
}

void KemaiUpdater::KemaiUpdaterPrivate::onNamFinished(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "Error on update check:" << reply->errorString();
    }
    else
    {
        // Parse json
        auto jdoc = QJsonDocument::fromJson(reply->readAll());
        auto jobj = jdoc.object();

        auto newVersion = QVersionNumber::fromString(jobj.value("tag_name").toString());
        if (newVersion > currentChecksinceVersion)
        {
            VersionDetails vd;
            vd.vn = newVersion;
            vd.description = jobj.value("body").toString();
            vd.url = jobj.value("html_url").toString();

            emit mQ->checkFinished(vd);
        }
        else if (not silenceIfNoNew)
        {
            emit mQ->checkFinished(VersionDetails());
        }
    }
}

/*
 * Public impl
 */
KemaiUpdater::KemaiUpdater(QObject* parent) : QObject(parent), mD(new KemaiUpdaterPrivate(this)) {}

KemaiUpdater::~KemaiUpdater() {}

void KemaiUpdater::checkAvailableNewVersion(const QVersionNumber& sinceVersion, bool silenceIfNoNew)
{
    mD->currentChecksinceVersion = sinceVersion;
    mD->silenceIfNoNew = silenceIfNoNew;

    auto rq = mD->prepareGithubRequest("/repos/AlexandrePTJ/kemai/releases/latest");
    mD->networkAccessManager->get(rq);
}
