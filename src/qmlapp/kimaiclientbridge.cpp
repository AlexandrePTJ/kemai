#include "kimaiclientbridge.h"

#include <QDebug>

#include "client/kimairequestfactory.h"
#include "settings.h"

using namespace kemai::client;
using namespace kemai::core;
using namespace kemai::qmlapp;

KimaiClientBridge::KimaiClientBridge()
{
    connect(&mClient, &KimaiClient::requestError, this, &KimaiClientBridge::onClientError);
    connect(&mClient, &KimaiClient::replyReceived, this, &KimaiClientBridge::onClientReply);
    connect(&mSecondTimer, &QTimer::timeout, this, &KimaiClientBridge::timeSheetDurationChanged);

    mSecondTimer.setInterval(1000);
    mSecondTimer.setTimerType(Qt::PreciseTimer);
}

void KimaiClientBridge::refresh()
{
    // check if we have running timesheet.
    // if one is running, it will re-fill combobox one by one, and set correct values
    // else it will only fill customer
    mClient.sendRequest(KimaiRequestFactory::activeTimeSheets());

    mClient.sendRequest(KimaiRequestFactory::customers());
    mClient.sendRequest(KimaiRequestFactory::projects());
    mClient.sendRequest(KimaiRequestFactory::activities());

    mClient.sendRequest(KimaiRequestFactory::me());
    mClient.sendRequest(KimaiRequestFactory::tags());
}

QAbstractListModel* KimaiClientBridge::customerDataModel()
{
    return &mCustomerDataModel;
}

QAbstractListModel* KimaiClientBridge::projectDataModel()
{
    return &mProjectDataModel;
}

QAbstractListModel* KimaiClientBridge::activityDataModel()
{
    return &mActivityDataModel;
}

bool KimaiClientBridge::timeSheetRunning() const
{
    return !mCurrentTimeSheet.isNull();
}

QString KimaiClientBridge::timeSheetDuration() const
{
    if (mCurrentTimeSheet)
    {
        auto nsecs        = mCurrentTimeSheet->beginAt.secsTo(QDateTime::currentDateTime());
        auto durationTime = QTime(0, 0).addSecs(nsecs);
        return durationTime.toString();
    }
    else
    {
        return "--:--:--";
    }
}

void KimaiClientBridge::reloadClientSettings()
{
    const auto& settings = Settings::load();
    mClient.setHost(settings.kimai.host);
    mClient.setUsername(settings.kimai.username);
    mClient.setToken(settings.kimai.token);
    refresh();
}

void KimaiClientBridge::onClientError(const QString& errorMsg)
{
    qDebug() << errorMsg;
}

void KimaiClientBridge::onClientReply(const KimaiReply& reply)
{
    if (!reply.isValid())
        return;

    switch (reply.method())
    {
        //    case ApiMethod::MeUsers: {
        //        mMe.reset(new User(reply.get<User>()));
        //    }
        //    break;
        //
    case ApiMethod::Customers:
        mCustomerDataModel.setValues(reply.get<Customers>());
        break;

    case ApiMethod::Projects:
        mProjectDataModel.setValues(reply.get<Projects>());
        break;

    case ApiMethod::Activities:
        mActivityDataModel.setValues(reply.get<Activities>());
        break;

    case ApiMethod::ActiveTimeSheets: {
        const auto& timeSheets = reply.get<TimeSheets>();

        if (!timeSheets.empty())
        {
            const auto currentTimeSheet = timeSheets.first();
            if (mCurrentTimeSheet)
            {
                if (mCurrentTimeSheet->id == currentTimeSheet.id)
                {
                    return;
                }
            }

            mCurrentTimeSheet.reset(new TimeSheet(currentTimeSheet));
            mSecondTimer.start();
            emit timeSheetRunningChanged();
        }
        else if (timeSheets.empty() && !mCurrentTimeSheet)
        {
            mCurrentTimeSheet.reset(nullptr);
            mSecondTimer.stop();
            emit timeSheetRunningChanged();
        }
    }
    break;

        //    case ApiMethod::TimeSheets: {
        //        auto ts = reply.get<TimeSheet>();
        //        if (not ts.endAt.isValid())
        //        {
        //            mCurrentTimeSheet.reset(new TimeSheet(ts));
        //            mUi->dteStartedAt->setDateTime(mCurrentTimeSheet->beginAt);
        //        }
        //        else
        //        {
        //            mCurrentTimeSheet.reset(nullptr);
        //        }
        //        updateControls();
        //    }
        //    break;

    case ApiMethod::Tags: {
        auto tags = reply.get<Tags>();
    }

    default:
        break;
    }
}
