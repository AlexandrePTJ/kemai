#pragma once

#include <QObject>
#include <QTimer>

#include "client/kimaiapi.h"
#include "client/kimaiclient.h"

#include "kimaibasedatamodel.h"

namespace kemai::qmlapp {

class KimaiClientBridge : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool timeSheetRunning READ timeSheetRunning NOTIFY timeSheetRunningChanged)
    Q_PROPERTY(QString timeSheetDuration READ timeSheetDuration NOTIFY timeSheetDurationChanged)

public:
    KimaiClientBridge();
    virtual ~KimaiClientBridge() = default;

    Q_INVOKABLE void refresh();

    QAbstractListModel* customerDataModel();
    QAbstractListModel* projectDataModel();
    QAbstractListModel* activityDataModel();

    bool timeSheetRunning() const;
    QString timeSheetDuration() const;

public slots:
    void reloadClientSettings();

signals:
    void timeSheetRunningChanged();
    void timeSheetDurationChanged();

private slots:
    void onClientError(const QString& errorMsg);
    void onClientReply(const client::KimaiReply& reply);

private:
    client::KimaiClient mClient;
    QTimer mSecondTimer;
    QScopedPointer<client::TimeSheet> mCurrentTimeSheet;
    KimaiBaseDataModel<client::Customer> mCustomerDataModel;
    KimaiBaseDataModel<client::Project> mProjectDataModel;
    KimaiBaseDataModel<client::Activity> mActivityDataModel;
};

} // namespace kemai::qmlapp
