#include "activityWidget.h"
#include "ui_activityWidget.h"

#include <spdlog/spdlog.h>

#include "activityDialog.h"
#include "customerDialog.h"
#include "projectDialog.h"

using namespace kemai;

ActivityWidget::ActivityWidget(QWidget* parent) : QWidget(parent), mUi(new Ui::ActivityWidget)
{
    mUi->setupUi(this);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    connect(mUi->cbCustomer, &QComboBox::currentTextChanged, this, [this](const QString&) { onCbCustomerFieldChanged(); });
    connect(mUi->cbCustomer,  qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int) { onCbCustomerFieldChanged(); });
    connect(mUi->cbProject, &QComboBox::currentTextChanged, this, [this](const QString&) { onCbProjectFieldChanged(); });
    connect(mUi->cbProject, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int) { onCbProjectFieldChanged(); });
    connect(mUi->cbActivity, &QComboBox::currentTextChanged, this, [this](const QString&) { onCbActivityFieldChanged(); });
    connect(mUi->cbActivity, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int) { onCbActivityFieldChanged(); });
#else
    connect(mUi->cbCustomer, &QComboBox::currentTextChanged, this, &ActivityWidget::onCbCustomerFieldChanged);
    connect(mUi->cbCustomer, &QComboBox::currentIndexChanged, this, &ActivityWidget::onCbCustomerFieldChanged);
    connect(mUi->cbProject, &QComboBox::currentTextChanged, this, &ActivityWidget::onCbProjectFieldChanged);
    connect(mUi->cbProject, &QComboBox::currentIndexChanged, this, &ActivityWidget::onCbProjectFieldChanged);
    connect(mUi->cbActivity, &QComboBox::currentTextChanged, this, &ActivityWidget::onCbActivityFieldChanged);
    connect(mUi->cbActivity, &QComboBox::currentIndexChanged, this, &ActivityWidget::onCbActivityFieldChanged);
#endif
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
        connect(&mSession->cache(), &KimaiCache::synchronizeFinished, this, &ActivityWidget::onSessionCacheSynchronizeFinished);
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

void ActivityWidget::onCbCustomerFieldChanged()
{
    updateProjectsCombo();
    updateControls();
}

void ActivityWidget::onCbProjectFieldChanged()
{
    updateActivitiesCombo();
    updateControls();
}

void ActivityWidget::onCbActivityFieldChanged()
{
    if (mSession)
    {
        if (!mSession->hasCurrentTimeSheet())
        {
            mUi->pteDescription->clear();
            mUi->leTags->clear();
        }
    }
    updateControls();
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

        // NOLINTBEGIN(readability-magic-numbers)
        const auto nDays = nSecs / 86400;
        nSecs -= nDays * 86400;

        const auto nHours = nSecs / 3600;
        nSecs -= nHours * 3600;

        const auto nMinutes = nSecs / 60;
        nSecs -= nMinutes * 60;

        mUi->lbDurationTime->setText(QString("%1%2:%3:%4")
                                         .arg(nDays > 0 ? QString::number(nDays) + "d " : "")
                                         .arg(nHours, 2, 10, QChar('0'))
                                         .arg(nMinutes, 2, 10, QChar('0'))
                                         .arg(nSecs, 2, 10, QChar('0')));
        // NOLINTEND(readability-magic-numbers)
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

void ActivityWidget::onSessionCacheSynchronizeFinished()
{
    mUi->cbCustomer->setKimaiData(mSession->cache().customers());
    mUi->cbProject->setKimaiData(mSession->cache().projects());
    mUi->cbActivity->setKimaiData(mSession->cache().activities());
    setEnabled(true);

    // Update all fields in case cache have refreshed with a running timesheet
    if (mSession->hasCurrentTimeSheet())
    {
        onSessionCurrentTimeSheetChanged();
    }
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

    auto noTimeSheetRunning = !mSession->hasCurrentTimeSheet();

    mUi->cbCustomer->setEnabled(noTimeSheetRunning);
    mUi->cbProject->setEnabled(noTimeSheetRunning);
    mUi->cbActivity->setEnabled(noTimeSheetRunning && !mUi->cbProject->currentText().isEmpty());
    mUi->tbEnableStartedAtEdit->setEnabled(noTimeSheetRunning);
    mUi->dteStartedAt->setEnabled(noTimeSheetRunning && mUi->tbEnableStartedAtEdit->isChecked());

    mUi->tbAddCustomer->setEnabled(noTimeSheetRunning);
    mUi->tbAddProject->setEnabled(noTimeSheetRunning && !mUi->cbCustomer->currentText().isEmpty());

    bool projectOk = mUi->cbCustomer->currentText().isEmpty() || (!mUi->cbCustomer->currentText().isEmpty() && !mUi->cbProject->currentText().isEmpty());
    mUi->tbAddActivity->setEnabled(noTimeSheetRunning && projectOk);

    bool enableStartStop = !mUi->cbProject->currentText().isEmpty() && !mUi->cbActivity->currentText().isEmpty();
    mUi->btStartStop->setEnabled(enableStartStop);

    if (noTimeSheetRunning)
    {
        mUi->btStartStop->setIcon(QIcon(":/icons/play"));
        mUi->lbDurationTime->clear();
    }
    else
    {
        mUi->btStartStop->setIcon(QIcon(":/icons/stop"));
    }

    emit currentActivityChanged(noTimeSheetRunning);
}

void ActivityWidget::updateCustomersCombo()
{
    if (mSession)
    {
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
    if (mSession)
    {
        // When no customer selected, list all projects
        auto customerId = mUi->cbCustomer->currentText().isEmpty() ? std::nullopt : std::optional<int>(mUi->cbCustomer->currentData().toInt());
        mUi->cbProject->setFilter(mSession->cache().projects(customerId));

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
    if (mSession)
    {
        auto projectId = mUi->cbProject->currentText().isEmpty() ? std::nullopt : std::optional<int>(mUi->cbProject->currentData().toInt());
        mUi->cbActivity->setFilter(mSession->cache().activities(projectId));

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
        else if (!projectId.has_value())
        {
            mUi->cbActivity->setCurrentText("");
        }
    }
}
