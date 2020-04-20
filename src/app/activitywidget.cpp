#include "activitywidget.h"
#include "ui_activitywidget.h"

#include "settings.h"

#include "kemai/kimairequestfactory.h"

#include <QDebug>

using namespace kemai::app;
using namespace kemai::client;

ActivityWidget::ActivityWidget(QWidget* parent) : QWidget(parent), mUi(new Ui::ActivityWidget)
{
    mUi->setupUi(this);

    connect(mUi->cbCustomer, &QComboBox::currentTextChanged, this, &ActivityWidget::onCbCustomerTextChanged);
    connect(mUi->cbProject, &QComboBox::currentTextChanged, this, &ActivityWidget::onCbProjectTextChanged);
    connect(mUi->cbActivity, &QComboBox::currentTextChanged, this, &ActivityWidget::onCbActivityTextChanged);
    connect(mUi->btStartStop, &QPushButton::clicked, this, &ActivityWidget::onBtStartStopClicked);
    connect(&mSecondTimer, &QTimer::timeout, this, &ActivityWidget::onSecondTimeout);

    mSecondTimer.setInterval(1000);
    mSecondTimer.setTimerType(Qt::PreciseTimer);
    mSecondTimer.start();
}

ActivityWidget::~ActivityWidget()
{
    mSecondTimer.stop();
    delete mUi;
};

void ActivityWidget::refresh()
{
    mUi->cbActivity->clear();
    mUi->cbProject->clear();
    mUi->cbCustomer->clear();

    // reset client
    auto settings = Settings::load();
    if (settings.isReady())
    {
        mClient.reset(new KimaiClient);
        mClient->setHost(settings.host);
        mClient->setUsername(settings.username);
        mClient->setToken(settings.token);

        connect(mClient.data(), &KimaiClient::requestError, this, &ActivityWidget::onClientError);
        connect(mClient.data(), &KimaiClient::replyReceived, this, &ActivityWidget::onClientReply);

        // check if we have running timesheet.
        // if one is running, it will re-fill combobox one by one, and set correct values
        // else it will only fill customer
        mClient->sendRequest(KimaiRequestFactory::activeTimeSheets());
    }
    else
    {
        mClient.reset(nullptr);
    }

    setEnabled(mClient != nullptr);
}

void ActivityWidget::onClientError(const QString& errorMsg)
{
    qDebug() << errorMsg;
}

void ActivityWidget::onClientReply(const KimaiReply& reply)
{
    switch (reply.method())
    {
    case ApiMethod::Customers: {
        mUi->cbCustomer->addItem("");

        for (const auto& customer : reply.get<Customers>())
            mUi->cbCustomer->addItem(customer.name, customer.id);

        if (mCurrentTimeSheet)
            mUi->cbCustomer->setCurrentText(mCurrentTimeSheet->project.customer.name);
    }
    break;

    case ApiMethod::Projects: {
        mUi->cbProject->addItem("");

        for (const auto& project : reply.get<Projects>())
            mUi->cbProject->addItem(project.name, project.id);

        if (mCurrentTimeSheet)
            mUi->cbProject->setCurrentText(mCurrentTimeSheet->project.name);
    }
    break;

    case ApiMethod::Activities: {
        mUi->cbActivity->addItem("");

        for (const auto& activity : reply.get<Activities>())
            mUi->cbActivity->addItem(activity.name, activity.id);

        if (mCurrentTimeSheet)
            mUi->cbActivity->setCurrentText(mCurrentTimeSheet->activity.name);
    }
    break;

    case ApiMethod::ActiveTimeSheets: {
        const auto& timeSheets = reply.get<TimeSheets>();

        if (not timeSheets.empty())
        {
            mCurrentTimeSheet.reset(new TimeSheet(timeSheets.first()));
            mUi->dteStartedAt->setDateTime(mCurrentTimeSheet->beginAt);
        }
        else
        {
            mCurrentTimeSheet.reset(nullptr);
        }

        updateControls();

        mClient->sendRequest(KimaiRequestFactory::customers());
    }
    break;

    case ApiMethod::TimeSheets: {
        auto ts = reply.get<TimeSheet>();
        if (not ts.endAt.isValid())
        {
            mCurrentTimeSheet.reset(new TimeSheet(ts));
            mUi->dteStartedAt->setDateTime(mCurrentTimeSheet->beginAt);
        }
        else
        {
            mCurrentTimeSheet.reset(nullptr);
        }
        updateControls();
    }
    break;

    default:
        break;
    }
}

void ActivityWidget::onCbCustomerTextChanged(const QString& text)
{
    mUi->cbProject->clear();

    if (not text.isEmpty())
    {
        auto customerId = mUi->cbCustomer->currentData().toInt();
        mClient->sendRequest(KimaiRequestFactory::projects(customerId));
    }
}

void ActivityWidget::onCbProjectTextChanged(const QString& text)
{
    mUi->cbActivity->clear();

    if (not text.isEmpty())
    {
        auto projectId = mUi->cbProject->currentData().toInt();
        mClient->sendRequest(KimaiRequestFactory::activities(projectId));
    }
}

void ActivityWidget::onCbActivityTextChanged(const QString& text) {}

void ActivityWidget::onSecondTimeout()
{
    if (mCurrentTimeSheet)
    {
        auto nsecs        = mCurrentTimeSheet->beginAt.secsTo(QDateTime::currentDateTime());
        auto durationTime = QTime(0, 0).addSecs(nsecs);
        mUi->lbDurationTime->setText(durationTime.toString());
    }
    else
    {
        mUi->dteStartedAt->setDateTime(QDateTime::currentDateTime());
    }
}

void ActivityWidget::onBtStartStopClicked()
{
    if (mCurrentTimeSheet)
    {
        mClient->sendRequest(KimaiRequestFactory::stopTimeSheet(mCurrentTimeSheet->id));
    }
    else
    {
        auto beginAt    = QDateTime::currentDateTime();
        auto projectId  = mUi->cbProject->currentData().toInt();
        auto activityId = mUi->cbActivity->currentData().toInt();
        mClient->sendRequest(KimaiRequestFactory::startTimeSheet(projectId, activityId, beginAt));
    }
}

void ActivityWidget::updateControls()
{
    auto enable = mCurrentTimeSheet.isNull();
    mUi->cbCustomer->setEnabled(enable);
    mUi->cbProject->setEnabled(enable);
    mUi->cbActivity->setEnabled(enable);

    if (enable)
    {
        mUi->btStartStop->setText(tr("Start"));
        mUi->btStartStop->setIcon(QIcon(":/icons/play"));
        mUi->lbDurationTime->clear();
    }
    else
    {
        mUi->btStartStop->setText(tr("Stop"));
        mUi->btStartStop->setIcon(QIcon(":/icons/stop"));
    }
}
