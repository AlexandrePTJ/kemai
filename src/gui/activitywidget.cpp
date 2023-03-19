#include "activitywidget.h"
#include "ui_activitywidget.h"

#include <spdlog/spdlog.h>

#include "activitydialog.h"
#include "customerdialog.h"
#include "projectdialog.h"
#include "settings/settings.h"

using namespace kemai;

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

    updateControls();
}

ActivityWidget::~ActivityWidget()
{
    mSecondTimer.stop();
    delete mUi;
}

void ActivityWidget::setKemaiSession(std::shared_ptr<KemaiSession> kemaiSession)
{
    mSession = std::move(kemaiSession);

    mUi->cbActivity->clear();
    mUi->cbProject->clear();
    mUi->cbCustomer->clear();

    if (mSession)
    {
        connect(mSession.get(), &KemaiSession::currentTimeSheetChanged, this, &ActivityWidget::onSessionCurrentTimeSheetChanged);
        connect(&mSession->cache(), &KimaiCache::synchronizeStarted, this, [this]() { setEnabled(false); });
        connect(&mSession->cache(), &KimaiCache::synchronizeFinished, this, [this]() { setEnabled(true); });
    }
    else
    {
        setEnabled(false);
    }
}

void ActivityWidget::stopCurrentTimeSheet()
{
    if (mSession)
    {
        if (mSession->hasCurrentTimeSheet())
        {
            auto timeSheet = mSession->currentTimeSheet().value();

            timeSheet.endAt       = mSession->computeTZDateTime(QDateTime::currentDateTime());
            timeSheet.description = mUi->pteDescription->toPlainText();
            timeSheet.tags        = mUi->leTags->text().split(',', Qt::SkipEmptyParts);

            auto timeSheetResult = mSession->client()->updateTimeSheet(timeSheet, mSession->timeSheetConfig().trackingMode);

            connect(timeSheetResult, &KimaiApiBaseResult::ready, this, [this, timeSheetResult] {
                mSession->refreshCurrentTimeSheet();
                timeSheetResult->deleteLater();
            });
            connect(timeSheetResult, &KimaiApiBaseResult::error, [timeSheetResult]() { timeSheetResult->deleteLater(); });
        }
    }
}

void ActivityWidget::onCbCustomerTextChanged(const QString& /*text*/)
{
    updateProjectsCombo();
    updateControls();
}

void ActivityWidget::onCbProjectTextChanged(const QString& /*text*/)
{
    updateActivitiesCombo();
    updateControls();
}

void ActivityWidget::onCbActivityTextChanged(const QString& /*text*/)
{
    if (mSession)
    {
        if (!mSession->hasCurrentTimeSheet())
        {
            mUi->pteDescription->clear();
            mUi->leTags->clear();
        }
    }
}

void ActivityWidget::onTbAddCustomerClicked()
{
    auto dialog = CustomerDialog(this);
    if (dialog.exec() == QDialog::Accepted)
    {
        const auto& customer = dialog.customer();

        auto customerAddResult = mSession->client()->addCustomer(customer);
        connect(customerAddResult, &KimaiApiBaseResult::ready, this, [this, customerAddResult] {
            const auto& customer = customerAddResult->getResult();
            mUi->cbCustomer->addItem(customer.name, customer.id);
            customerAddResult->deleteLater();
            mSession->refreshCache({KimaiCache::Category::Customers});
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

        auto projectAddResult = mSession->client()->addProject(project);
        connect(projectAddResult, &KimaiApiBaseResult::ready, this, [this, projectAddResult] {
            const auto& project = projectAddResult->getResult();
            mUi->cbProject->addItem(project.name, project.id);
            projectAddResult->deleteLater();
            mSession->refreshCache({KimaiCache::Category::Projects});
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

        auto activityAddResult = mSession->client()->addActivity(activity);
        connect(activityAddResult, &KimaiApiBaseResult::ready, this, [this, activityAddResult] {
            const auto& activity = activityAddResult->getResult();
            mUi->cbActivity->addItem(activity.name, activity.id);
            activityAddResult->deleteLater();
            mSession->refreshCache({KimaiCache::Category::Activities});
        });
        connect(activityAddResult, &KimaiApiBaseResult::error, [activityAddResult]() { activityAddResult->deleteLater(); });
    }
}

void ActivityWidget::onSecondTimeout()
{
    const auto& now = QDateTime::currentDateTime();
    if (mSession && mSession->hasCurrentTimeSheet())
    {
        auto nSecs = mSession->currentTimeSheet()->beginAt.secsTo(now);

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

void ActivityWidget::onSessionCurrentTimeSheetChanged()
{
    // Waiting for cache to be filled before trying to update combos
    while (mSession->cache().status() != KimaiCache::Status::Ready)
    {
        qApp->processEvents();
    }

    if (mSession->hasCurrentTimeSheet())
    {
        mUi->dteStartedAt->setDateTime(mSession->currentTimeSheet()->beginAt);
        mUi->pteDescription->setPlainText(mSession->currentTimeSheet()->description);
        mUi->leTags->setText(mSession->currentTimeSheet()->tags.join(','));
    }

    updateControls();
    updateCustomersCombo();
}

void ActivityWidget::onBtStartStopClicked()
{
    if (mSession->hasCurrentTimeSheet())
    {
        stopCurrentTimeSheet();
    }
    else
    {
        TimeSheet timeSheet;

        timeSheet.beginAt     = mSession->computeTZDateTime(mUi->dteStartedAt->dateTime());
        timeSheet.project.id  = mUi->cbProject->currentData().toInt();
        timeSheet.activity.id = mUi->cbActivity->currentData().toInt();
        timeSheet.description = mUi->pteDescription->toPlainText();
        timeSheet.tags        = mUi->leTags->text().split(',', Qt::SkipEmptyParts);

        auto timeSheetResult = mSession->client()->startTimeSheet(timeSheet, mSession->timeSheetConfig().trackingMode);

        connect(timeSheetResult, &KimaiApiBaseResult::ready, this, [this, timeSheetResult] {
            mSession->refreshCurrentTimeSheet();
            timeSheetResult->deleteLater();
        });
        connect(timeSheetResult, &KimaiApiBaseResult::error, [timeSheetResult]() { timeSheetResult->deleteLater(); });
    }
}

void ActivityWidget::updateControls()
{
    // Can be raised while clearing combobox on session changes
    if (!mSession)
    {
        setEnabled(false);
        return;
    }

    auto enable = !mSession->hasCurrentTimeSheet();

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

void ActivityWidget::updateCustomersCombo()
{
    mUi->cbCustomer->clear();

    if (mSession)
    {
        auto customers = mSession->cache().customers();
        if (!customers.empty())
        {
            mUi->cbCustomer->clear();
            mUi->cbCustomer->addItem("");
            for (const auto& customer : customers)
            {
                mUi->cbCustomer->addItem(customer.name, customer.id);
            }
        }

        if (mSession->hasCurrentTimeSheet())
        {
            auto customerIndex = mUi->cbCustomer->findData(mSession->currentTimeSheet()->project.customer.id);
            if (customerIndex >= 0)
            {
                mUi->cbCustomer->setCurrentIndex(customerIndex);
            }
            else
            {
                spdlog::error("Cannot find '{}' customer", mSession->currentTimeSheet()->project.customer.name.toStdString());
            }
        }
    }
}

void ActivityWidget::updateProjectsCombo()
{
    mUi->cbProject->clear();

    if (mSession)
    {
        // When no customer selected, list all projects
        auto customerId = mUi->cbCustomer->currentText().isEmpty() ? std::nullopt : std::optional<int>(mUi->cbCustomer->currentData().toInt());
        auto projects   = mSession->cache().projects(customerId);

        if (!projects.empty())
        {
            mUi->cbProject->addItem("");

            for (const auto& project : projects)
            {
                mUi->cbProject->addItem(project.name, project.id);
            }
        }

        if (mSession->hasCurrentTimeSheet())
        {
            auto projectIndex = mUi->cbProject->findData(mSession->currentTimeSheet()->project.id);
            if (projectIndex >= 0)
            {
                mUi->cbProject->setCurrentIndex(projectIndex);
            }
            else
            {
                spdlog::error("Cannot find '{}' project", mSession->currentTimeSheet()->project.name.toStdString());
            }
        }
    }
}

void ActivityWidget::updateActivitiesCombo()
{
    mUi->cbActivity->clear();

    if (!mUi->cbProject->currentText().isEmpty() && mSession)
    {
        auto projectId  = mUi->cbProject->currentData().toInt();
        auto activities = mSession->cache().activities(projectId);

        if (!activities.empty())
        {
            mUi->cbActivity->clear();
            mUi->cbActivity->addItem("");

            for (const auto& activity : activities)
            {
                mUi->cbActivity->addItem(activity.name, activity.id);
            }
        }

        if (mSession->hasCurrentTimeSheet())
        {
            auto activityIndex = mUi->cbActivity->findData(mSession->currentTimeSheet()->activity.id);
            if (activityIndex >= 0)
            {
                mUi->cbActivity->setCurrentIndex(activityIndex);
            }
            else
            {
                spdlog::error("Cannot find '{}' activity", mSession->currentTimeSheet()->activity.name.toStdString());
            }
        }
    }
}
