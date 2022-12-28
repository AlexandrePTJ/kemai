#include "activitywidget.h"
#include "ui_activitywidget.h"

#include "activitydialog.h"
#include "customerdialog.h"
#include "projectdialog.h"
#include "settings.h"

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

    mSecondTimer.setInterval(std::chrono::seconds(1));
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
        // check if we have running timesheet.
        // if one is running, it will re-fill combobox one by one, and set correct values
        // else it will only fill customer
        auto activeTimeSheetsResult = mClient->requestActiveTimeSheets();
        connect(activeTimeSheetsResult, &KimaiApiBaseResult::ready, this,
                [this, activeTimeSheetsResult] { processActiveTimeSheetsResult(activeTimeSheetsResult); });
        connect(activeTimeSheetsResult, &KimaiApiBaseResult::error, [activeTimeSheetsResult]() { activeTimeSheetsResult->deleteLater(); });
    }

    setEnabled(mClient != nullptr);
}

void ActivityWidget::setKemaiSession(QSharedPointer<core::KemaiSession> kemaiSession)
{
    mSession = std::move(kemaiSession);
}

void ActivityWidget::onCbCustomerTextChanged(const QString& text)
{
    mUi->cbProject->clear();

    if (!text.isEmpty())
    {
        auto customerId = mUi->cbCustomer->currentData().toInt();

        auto projectsResult = mClient->requestProjects(customerId);
        connect(projectsResult, &KimaiApiBaseResult::ready, this, [this, projectsResult] { processProjectsResult(projectsResult); });
        connect(projectsResult, &KimaiApiBaseResult::error, [projectsResult]() { projectsResult->deleteLater(); });
    }
    updateControls();
}

void ActivityWidget::onCbProjectTextChanged(const QString& text)
{
    mUi->cbActivity->clear();

    if (!text.isEmpty())
    {
        auto projectId = mUi->cbProject->currentData().toInt();

        auto activitiesResult = mClient->requestActivities(projectId);
        connect(activitiesResult, &KimaiApiBaseResult::ready, this, [this, activitiesResult] { processActivitiesResult(activitiesResult); });
        connect(activitiesResult, &KimaiApiBaseResult::error, [activitiesResult]() { activitiesResult->deleteLater(); });
    }
    updateControls();
}

void ActivityWidget::onCbActivityTextChanged(const QString& /*text*/)
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

        auto customerAddResult = mClient->addCustomer(customer);
        connect(customerAddResult, &KimaiApiBaseResult::ready, this, [this, customerAddResult] {
            const auto& customer = customerAddResult->getResult();
            mUi->cbCustomer->addItem(customer.name, customer.id);
            customerAddResult->deleteLater();
        });
        connect(customerAddResult, &KimaiApiBaseResult::error, [customerAddResult]() { customerAddResult->deleteLater(); });
    }
}

void ActivityWidget::onTbAddProjectClicked()
{
    auto dialog = ProjectDialog(this);
    if (dialog.exec() == QDialog::Accepted)
    {
        auto project        = dialog.project();
        project.customer.id = mUi->cbCustomer->currentData().toInt();

        auto projectAddResult = mClient->addProject(project);
        connect(projectAddResult, &KimaiApiBaseResult::ready, this, [this, projectAddResult] {
            const auto& project = projectAddResult->getResult();
            mUi->cbProject->addItem(project.name, project.id);
            projectAddResult->deleteLater();
        });
        connect(projectAddResult, &KimaiApiBaseResult::error, [projectAddResult]() { projectAddResult->deleteLater(); });
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

        auto activityAddResult = mClient->addActivity(activity);
        connect(activityAddResult, &KimaiApiBaseResult::ready, this, [this, activityAddResult] {
            const auto& activity = activityAddResult->getResult();
            mUi->cbActivity->addItem(activity.name, activity.id);
            activityAddResult->deleteLater();
        });
        connect(activityAddResult, &KimaiApiBaseResult::error, [activityAddResult]() { activityAddResult->deleteLater(); });
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
    TimeSheetResult timeSheetResult = nullptr;

    if (mCurrentTimeSheet)
    {
        timeSheetResult = mClient->stopTimeSheet(*mCurrentTimeSheet);
    }
    else
    {
        TimeSheet timeSheet;

        timeSheet.beginAt = mUi->dteStartedAt->dateTime();

        // Be sure to use expected timezone
        auto timeZone = QTimeZone(mSession->me.timezone.toLocal8Bit());
        if (timeZone.isValid())
        {
            timeSheet.beginAt = timeSheet.beginAt.toTimeZone(timeZone);
        }

        timeSheet.project.id  = mUi->cbProject->currentData().toInt();
        timeSheet.activity.id = mUi->cbActivity->currentData().toInt();
        timeSheet.description = mUi->pteDescription->toPlainText();
        timeSheet.tags        = mUi->leTags->text().split(',', Qt::SkipEmptyParts);

        timeSheetResult = mClient->startTimeSheet(timeSheet, mSession->timeSheetConfig.trackingMode);
    }

    if (timeSheetResult != nullptr)
    {
        connect(timeSheetResult, &KimaiApiBaseResult::ready, this, [this, timeSheetResult] { processTimeSheetResult(timeSheetResult); });
        connect(timeSheetResult, &KimaiApiBaseResult::error, [timeSheetResult]() { timeSheetResult->deleteLater(); });
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

void ActivityWidget::processActiveTimeSheetsResult(TimeSheetsResult timeSheetsResult)
{
    const auto& timeSheets = timeSheetsResult->getResult();

    if (!timeSheets.empty())
    {
        mCurrentTimeSheet.reset(new TimeSheet(timeSheets.at(0)));
        mUi->dteStartedAt->setDateTime(mCurrentTimeSheet->beginAt);
        mUi->pteDescription->setPlainText(mCurrentTimeSheet->description);
        mUi->leTags->setText(mCurrentTimeSheet->tags.join(','));
    }
    else
    {
        mCurrentTimeSheet.reset(nullptr);
    }

    updateControls();

    auto customersResult = mClient->requestCustomers();
    connect(customersResult, &KimaiApiBaseResult::ready, this, [this, customersResult] { processCustomersResult(customersResult); });
    connect(customersResult, &KimaiApiBaseResult::error, [customersResult]() { customersResult->deleteLater(); });

    timeSheetsResult->deleteLater();
}

void ActivityWidget::processCustomersResult(CustomersResult customersResult)
{
    const auto& customers = customersResult->getResult();
    if (!customers.empty())
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
        auto customerIndex = mUi->cbCustomer->findData(mCurrentTimeSheet->project.customer.id);
        if (customerIndex >= 0)
        {
            mUi->cbCustomer->setCurrentIndex(customerIndex);
        }
        else
        {
            spdlog::error("Cannot find '{}' customer", mCurrentTimeSheet->project.customer.name.toStdString());
        }
    }
    customersResult->deleteLater();
}

void ActivityWidget::processProjectsResult(ProjectsResult projectsResult)
{
    const auto& projects = projectsResult->getResult();
    if (!projects.empty())
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
        auto projectIndex = mUi->cbProject->findData(mCurrentTimeSheet->project.id);
        if (projectIndex >= 0)
        {
            mUi->cbProject->setCurrentIndex(projectIndex);
        }
        else
        {
            spdlog::error("Cannot find '{}' project", mCurrentTimeSheet->project.name.toStdString());
        }
    }

    projectsResult->deleteLater();
}

void ActivityWidget::processActivitiesResult(ActivitiesResult activitiesResult)
{
    const auto& activities = activitiesResult->getResult();
    if (!activities.empty())
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
        auto activityIndex = mUi->cbActivity->findData(mCurrentTimeSheet->activity.id);
        if (activityIndex >= 0)
        {
            mUi->cbActivity->setCurrentIndex(activityIndex);
        }
        else
        {
            spdlog::error("Cannot find '{}' activity", mCurrentTimeSheet->activity.name.toStdString());
        }
    }
    activitiesResult->deleteLater();
}

void ActivityWidget::processTimeSheetResult(TimeSheetResult timeSheetResult)
{
    auto timeSheet = timeSheetResult->getResult();
    if (!timeSheet.endAt.isValid())
    {
        mCurrentTimeSheet.reset(new TimeSheet(timeSheet));
        mUi->dteStartedAt->setDateTime(mCurrentTimeSheet->beginAt);
    }
    else
    {
        mCurrentTimeSheet.reset();
    }
    updateControls();
    timeSheetResult->deleteLater();
}
