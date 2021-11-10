#include "activitywidget.h"
#include "ui_activitywidget.h"

#include "activitydialog.h"
#include "customerdialog.h"
#include "projectdialog.h"
#include "settings.h"

#include "client/kimairequestfactory.h"

#include <QTimeZone>

#include <spdlog/spdlog.h>

using namespace kemai::app;
using namespace kemai::client;
using namespace kemai::core;

ActivityWidget::ActivityWidget(QWidget* parent) : QWidget(parent), mUi(new Ui::ActivityWidget)
{
    mUi->setupUi(this);

    connect(mUi->cbCustomer, &QComboBox::currentTextChanged, this, &ActivityWidget::onCbCustomerTextChanged);
    connect(mUi->cbProject, &QComboBox::currentTextChanged, this, &ActivityWidget::onCbProjectTextChanged);
    connect(mUi->cbActivity, &QComboBox::currentTextChanged, this, &ActivityWidget::onCbActivityTextChanged);
    connect(mUi->tbAddCustomer, &QToolButton::clicked, this, &ActivityWidget::onTbAddCustomerClicked);
    connect(mUi->tbAddProject, &QToolButton::clicked, this, &ActivityWidget::onTbAddProjectClicked);
    connect(mUi->tbAddActivity, &QToolButton::clicked, this, &ActivityWidget::onTbAddActivityClicked);
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
}

void ActivityWidget::setKimaiClient(QSharedPointer<client::KimaiClient> kimaiClient)
{
    mUi->cbActivity->clear();
    mUi->cbProject->clear();
    mUi->cbCustomer->clear();

    mClient = std::move(kimaiClient);
    if (mClient)
    {
        connect(mClient.get(), &KimaiClient::replyReceived, this, &ActivityWidget::onClientReply);

        // check if we have running timesheet.
        // if one is running, it will re-fill combobox one by one, and set correct values
        // else it will only fill customer
        mClient->sendRequest(KimaiRequestFactory::activeTimeSheets());
        mClient->sendRequest(KimaiRequestFactory::tags());
    }

    setEnabled(mClient != nullptr);
}

void ActivityWidget::setKemaiSession(QSharedPointer<core::KemaiSession> kemaiSession)
{
    mSession = std::move(kemaiSession);
}

void ActivityWidget::onClientReply(const KimaiReply& reply)
{
    if (!reply.isValid())
    {
        return;
    }

    switch (reply.method())
    {
    case ApiMethod::Customers: {
        const auto& customers = reply.get<Customers>();
        if (!customers.isEmpty())
        {
            mUi->cbCustomer->clear();
            mUi->cbCustomer->addItem("");
            for (const auto& customer : customers)
            {
                mUi->cbCustomer->addItem(customer.name, customer.id);
            }
        }

        if (mCurrentTimeSheet)
        {
            mUi->cbCustomer->setCurrentText(mCurrentTimeSheet->project.customer.name);
        }
    }
    break;

    case ApiMethod::CustomerAdd: {
        const auto& customer = reply.get<Customer>();
        mUi->cbCustomer->addItem(customer.name, customer.id);
    }
    break;

    case ApiMethod::Projects: {
        const auto& projects = reply.get<Projects>();
        if (!projects.isEmpty())
        {
            mUi->cbProject->clear();
            mUi->cbProject->addItem("");

            for (const auto& project : projects)
            {
                mUi->cbProject->addItem(project.name, project.id);
            }
        }

        if (mCurrentTimeSheet)
        {
            mUi->cbProject->setCurrentText(mCurrentTimeSheet->project.name);
        }
    }
    break;

    case ApiMethod::ProjectAdd: {
        const auto& project = reply.get<Project>();
        mUi->cbProject->addItem(project.name, project.id);
    }
    break;

    case ApiMethod::Activities: {
        const auto& activities = reply.get<Activities>();
        if (!activities.isEmpty())
        {
            mUi->cbActivity->clear();
            mUi->cbActivity->addItem("");

            for (const auto& activity : activities)
            {
                mUi->cbActivity->addItem(activity.name, activity.id);
            }
        }

        if (mCurrentTimeSheet)
        {
            mUi->cbActivity->setCurrentText(mCurrentTimeSheet->activity.name);
        }
    }
    break;

    case ApiMethod::ActivityAdd: {
        const auto& activity = reply.get<Activity>();
        mUi->cbActivity->addItem(activity.name, activity.id);
    }
    break;

    case ApiMethod::ActiveTimeSheets: {
        const auto& timeSheets = reply.get<TimeSheets>();

        if (!timeSheets.empty())
        {
            mCurrentTimeSheet.reset(new TimeSheet(timeSheets.first()));
            mUi->dteStartedAt->setDateTime(mCurrentTimeSheet->beginAt);
            mUi->pteDescription->setPlainText(mCurrentTimeSheet->description);
            mUi->leTags->setText(mCurrentTimeSheet->tags.join(','));
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
        if (!ts.endAt.isValid())
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

    if (!text.isEmpty())
    {
        auto customerId = mUi->cbCustomer->currentData().toInt();
        mClient->sendRequest(KimaiRequestFactory::projects(customerId));
    }
    updateControls();
}

void ActivityWidget::onCbProjectTextChanged(const QString& text)
{
    mUi->cbActivity->clear();

    if (!text.isEmpty())
    {
        auto projectId = mUi->cbProject->currentData().toInt();
        mClient->sendRequest(KimaiRequestFactory::activities(projectId));
    }
    updateControls();
}

void ActivityWidget::onCbActivityTextChanged(const QString& text)
{
    if (!mCurrentTimeSheet)
    {
        mUi->pteDescription->clear();
        mUi->leTags->clear();
    }
}

void ActivityWidget::onTbAddCustomerClicked()
{
    auto dialog = CustomerDialog(this);
    if (dialog.exec() == QDialog::Accepted)
    {
        const auto& customer = dialog.customer();
        mClient->sendRequest(KimaiRequestFactory::customerAdd(customer));
    }
}

void ActivityWidget::onTbAddProjectClicked()
{
    auto dialog = ProjectDialog(this);
    if (dialog.exec() == QDialog::Accepted)
    {
        auto project        = dialog.project();
        project.customer.id = mUi->cbCustomer->currentData().toInt();
        mClient->sendRequest(KimaiRequestFactory::projectAdd(project));
    }
}

void ActivityWidget::onTbAddActivityClicked()
{
    auto dialog = ActivityDialog(this);
    if (dialog.exec() == QDialog::Accepted)
    {
        auto activity = dialog.activity();
        if (!mUi->cbCustomer->currentText().isEmpty())
        {
            Project project;
            project.customer.id = mUi->cbCustomer->currentData().toInt();
            project.id          = mUi->cbProject->currentData().toInt();

            activity.project = project;
        }
        mClient->sendRequest(KimaiRequestFactory::activityAdd(activity));
    }
}

void ActivityWidget::onSecondTimeout()
{
    const auto& now = QDateTime::currentDateTime();
    if (mCurrentTimeSheet)
    {
        auto nSecs = mCurrentTimeSheet->beginAt.secsTo(now);

        auto nDays = nSecs / 86400;
        nSecs -= nDays * 86400;

        auto nHours = nSecs / 3600;
        nSecs -= nHours * 3600;

        auto nMins = nSecs / 60;
        nSecs -= nMins * 60;

        mUi->lbDurationTime->setText(QString("%1%2:%3:%4")
                                         .arg(nDays > 0 ? QString::number(nDays) + "d " : "")
                                         .arg(nHours, 2, 10, QChar('0'))
                                         .arg(nMins, 2, 10, QChar('0'))
                                         .arg(nSecs, 2, 10, QChar('0')));
    }
    else
    {
        mUi->dteStartedAt->setMaximumDateTime(now);
        if (!mUi->dteStartedAt->isEnabled())
        {
            mUi->dteStartedAt->setDateTime(now);
        }
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
        auto beginAt = mUi->dteStartedAt->dateTime();

        // Be sure to use expected timezone
        auto tz = QTimeZone(mSession->me.timezone.toLocal8Bit());
        if (tz.isValid())
        {
            beginAt = beginAt.toTimeZone(tz);
        }

        auto projectId  = mUi->cbProject->currentData().toInt();
        auto activityId = mUi->cbActivity->currentData().toInt();
        auto desc       = mUi->pteDescription->toPlainText();
        auto tags       = mUi->leTags->text().split(',', Qt::SkipEmptyParts);

        mClient->sendRequest(KimaiRequestFactory::startTimeSheet(projectId, activityId, beginAt, desc, tags, mSession->timeSheetConfig.trackingMode));
    }
}

void ActivityWidget::updateControls()
{
    auto enable = mCurrentTimeSheet.isNull();
    mUi->cbCustomer->setEnabled(enable);
    mUi->cbProject->setEnabled(enable);
    mUi->cbActivity->setEnabled(enable);
    mUi->tbEnableStartedAtEdit->setEnabled(enable);
    mUi->dteStartedAt->setEnabled(enable && mUi->tbEnableStartedAtEdit->isChecked());

    mUi->tbAddCustomer->setEnabled(enable);
    mUi->tbAddProject->setEnabled(enable && !mUi->cbCustomer->currentText().isEmpty());

    bool projectOk = mUi->cbCustomer->currentText().isEmpty() || (!mUi->cbCustomer->currentText().isEmpty() && !mUi->cbProject->currentText().isEmpty());
    mUi->tbAddActivity->setEnabled(enable && projectOk);

    if (enable)
    {
        mUi->btStartStop->setIcon(QIcon(":/icons/play"));
        mUi->lbDurationTime->clear();
    }
    else
    {
        mUi->btStartStop->setIcon(QIcon(":/icons/stop"));
    }

    emit currentActivityChanged(enable);
}
