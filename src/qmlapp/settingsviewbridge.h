#pragma once

#include <QObject>

#include "client/kimaiclient.h"
#include "settings.h"

namespace kemai::qmlapp {

class SettingsViewBridge : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString host READ host WRITE setHost NOTIFY hostChanged)
    Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged)
    Q_PROPERTY(QString token READ token WRITE setToken NOTIFY tokenChanged)

public:
    SettingsViewBridge();
    virtual ~SettingsViewBridge() = default;

    void setHost(const QString& host);
    QString host() const;

    void setUsername(const QString& username);
    QString username() const;

    void setToken(const QString& token);
    QString token() const;

    Q_INVOKABLE void test();
    Q_INVOKABLE void save();

signals:
    void hostChanged(const QString& host);
    void usernameChanged(const QString& username);
    void tokenChanged(const QString& token);
    void testFinished(bool ok, const QString& message);

private slots:
    void onClientError(const QString& errorMsg);
    void onClientReply(const client::KimaiReply& reply);

private:
    client::KimaiClient mClient;
    core::Settings mSettings;
};

} // namespace kemai::qmlapp
