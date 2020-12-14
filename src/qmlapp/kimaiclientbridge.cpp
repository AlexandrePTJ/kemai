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
}

void KimaiClientBridge::refresh()
{
    // check if we have running timesheet.
    // if one is running, it will re-fill combobox one by one, and set correct values
    // else it will only fill customer
    mClient.sendRequest(KimaiRequestFactory::activeTimeSheets());
    mClient.sendRequest(KimaiRequestFactory::tags());
}

QAbstractListModel* KimaiClientBridge::customerDataModel()
{
    return &mCustomerDataModel;
}

bool KimaiClientBridge::timeSheetRunning() const
{
    return not mCurrentTimeSheet.isNull();
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
    if (not reply.isValid())
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

        //
        //    case ApiMethod::CustomerAdd: {
        //        const auto& customer = reply.get<Customer>();
        //        mUi->cbCustomer->addItem(customer.name, customer.id);
        //    }
        //    break;
        //
        //    case ApiMethod::Projects: {
        //        const auto& projects = reply.get<Projects>();
        //        if (not projects.isEmpty())
        //        {
        //            mUi->cbProject->clear();
        //            mUi->cbProject->addItem("");
        //
        //            for (const auto& project : projects)
        //                mUi->cbProject->addItem(project.name, project.id);
        //        }
        //
        //        if (mCurrentTimeSheet)
        //            mUi->cbProject->setCurrentText(mCurrentTimeSheet->project.name);
        //    }
        //    break;
        //
        //    case ApiMethod::ProjectAdd: {
        //        const auto& project = reply.get<Project>();
        //        mUi->cbProject->addItem(project.name, project.id);
        //    }
        //    break;
        //
        //    case ApiMethod::Activities: {
        //        const auto& activites = reply.get<Activities>();
        //        if (not activites.isEmpty())
        //        {
        //            mUi->cbActivity->clear();
        //            mUi->cbActivity->addItem("");
        //
        //            for (const auto& activity : activites)
        //                mUi->cbActivity->addItem(activity.name, activity.id);
        //        }
        //
        //        if (mCurrentTimeSheet)
        //            mUi->cbActivity->setCurrentText(mCurrentTimeSheet->activity.name);
        //    }
        //    break;
        //
        //    case ApiMethod::ActivityAdd: {
        //        const auto& activity = reply.get<Activity>();
        //        mUi->cbActivity->addItem(activity.name, activity.id);
        //    }
        //    break;

    case ApiMethod::ActiveTimeSheets: {
        const auto& timeSheets = reply.get<TimeSheets>();

        if (not timeSheets.empty())
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
            emit timeSheetRunningChanged(true);
        }
        else if (timeSheets.empty() and not mCurrentTimeSheet)
        {
            mCurrentTimeSheet.reset(nullptr);
            emit timeSheetRunningChanged(false);
        }

        mClient.sendRequest(KimaiRequestFactory::customers());
        mClient.sendRequest(KimaiRequestFactory::me());
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
