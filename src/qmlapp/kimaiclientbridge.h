#pragma once

#include <QObject>

#include "client/kimaiapi.h"
#include "client/kimaiclient.h"

#include "kimaibasedatamodel.h"

namespace kemai::qmlapp {

class KimaiClientBridge : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool timeSheetRunning READ timeSheetRunning NOTIFY timeSheetRunningChanged)

public:
    KimaiClientBridge();
    virtual ~KimaiClientBridge() = default;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE QAbstractListModel* customerDataModel();

    bool timeSheetRunning() const;

public slots:
    void reloadClientSettings();

signals:
    void timeSheetRunningChanged(bool running);

private slots:
    void onClientError(const QString& errorMsg);
    void onClientReply(const client::KimaiReply& reply);

private:
    client::KimaiClient mClient;
    QScopedPointer<client::TimeSheet> mCurrentTimeSheet;
    KimaiBaseDataModel<client::Customer> mCustomerDataModel;
};

} // namespace kemai::qmlapp
