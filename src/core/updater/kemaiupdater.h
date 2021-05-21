#pragma once

#include <QObject>
#include <QUrl>
#include <QVersionNumber>

namespace kemai::updater {

struct VersionDetails
{
    QVersionNumber vn;
    QString description;
    QUrl url;
};

class KemaiUpdater : public QObject
{
    Q_OBJECT

public:
    explicit KemaiUpdater(QObject* parent = nullptr);
    ~KemaiUpdater() override;

    void checkAvailableNewVersion(const QVersionNumber& sinceVersion = QVersionNumber(0, 0, 0), bool silenceIfNoNew = false);

signals:
    void checkFinished(const kemai::updater::VersionDetails& kv);

private:
    class KemaiUpdaterPrivate;
    QScopedPointer<KemaiUpdaterPrivate> mD;
};

} // namespace kemai::updater
