#include "activityWidget.h"
#include "ui_activityWidget.h"

#include <spdlog/spdlog.h>

#include "activityDialog.h"
#include "customerDialog.h"
#include "misc/customFmt.h"
#include "misc/helpers.h"
#include "projectDialog.h"
#include "timeSheetListWidgetItem.h"

using namespace kemai;

ActivityWidget::ActivityWidget(QWidget* parent) : QWidget(parent), mUi(std::make_unique<Ui::ActivityWidget>())
{
    mUi->setupUi(this);
    mUi->formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    connect(mUi->cbProject, &QComboBox::currentTextChanged, this, &ActivityWidget::onCbProjectFieldChanged);
    connect(mUi->cbProject, &QComboBox::currentIndexChanged, this, &ActivityWidget::onCbProjectFieldChanged);
    connect(mUi->cbActivity, &QComboBox::currentTextChanged, this, &ActivityWidget::onCbActivityFieldChanged);
    connect(mUi->cbActivity, &QComboBox::currentIndexChanged, this, &ActivityWidget::onCbActivityFieldChanged);
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
}

void ActivityWidget::setKemaiSession(std::shared_ptr<KemaiSession> kemaiSession)
{
    mSession = std::move(kemaiSession);

    mUi->cbActivity->clear();
    mUi->cbProject->clear();
    mUi->lwHistory->clear();

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

            timeSheet.endAt = mSession->computeTZDateTime(QDateTime::currentDateTime());

            auto timeSheetResult = mSession->client()->updateTimeSheet(timeSheet, mSession->timeSheetConfig().trackingMode);

            connect(timeSheetResult, &KimaiApiBaseResult::ready, this, [this, timeSheetResult] {
                if (mPendingStartRequest.has_value())
                {
                    startPendingTimeSheet();
                }
                else
                {
                    mSession->refreshCache(KimaiCache::Category::RecentTimeSheets);
                    mSession->refreshCurrentTimeSheet();
                }
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
        if (!mSession->hasCurrentTimeSheet()) {}
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
            customerAddResult->deleteLater();
            mSession->refreshCache(KimaiCache::Category::Customers);
        });
        connect(customerAddResult, &KimaiApiBaseResult::error, [customerAddResult]() { customerAddResult->deleteLater(); });
    }
}

void ActivityWidget::onTbAddProjectClicked()
{
    auto dialog = ProjectDialog(this);
    if (dialog.exec() == QDialog::Accepted)
    {
        auto project = dialog.project();

        auto projectAddResult = mSession->client()->addProject(project);
        connect(projectAddResult, &KimaiApiBaseResult::ready, this, [this, projectAddResult] {
            const auto& project = projectAddResult->getResult();
            mUi->cbProject->addItem(project.name, project.id);
            projectAddResult->deleteLater();
            mSession->refreshCache(KimaiCache::Category::Projects);
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

        auto activityAddResult = mSession->client()->addActivity(activity);
        connect(activityAddResult, &KimaiApiBaseResult::ready, this, [this, activityAddResult] {
            const auto& activity = activityAddResult->getResult();
            mUi->cbActivity->addItem(activity.name, activity.id);
            activityAddResult->deleteLater();
            mSession->refreshCache(KimaiCache::Category::Activities);
        });
        connect(activityAddResult, &KimaiApiBaseResult::error, [activityAddResult]() { activityAddResult->deleteLater(); });
    }
}

void ActivityWidget::onSecondTimeout()
{
    const auto& now = QDateTime::currentDateTime();
    if (mSession && mSession->hasCurrentTimeSheet())
    {
        mUi->lbDurationTime->setText(helpers::getDurationString(mSession->currentTimeSheet()->beginAt, now));
    }
    else {}
}

void ActivityWidget::onSessionCurrentTimeSheetChanged()
{
    // Waiting for cache to be filled before trying to update combos
    while (mSession->cache().status() != KimaiCache::Status::Ready)
    {
        qApp->processEvents();
    }

    if (mSession->hasCurrentTimeSheet()) {}

    updateControls();
    updateCustomersCombo();
}

void ActivityWidget::onSessionCacheSynchronizeFinished()
{
    mUi->cbProject->setKimaiData(mSession->cache().projects());
    mUi->cbActivity->setKimaiData(mSession->cache().activities());
    updateRecentTimeSheetsView();
    setEnabled(true);

    // Update all fields in case cache have refreshed with a running timesheet
    if (mSession->hasCurrentTimeSheet())
    {
        onSessionCurrentTimeSheetChanged();
    }
}

void ActivityWidget::onHistoryTimeSheetStartRequested(const TimeSheet& timeSheet)
{
    mPendingStartRequest = timeSheet;

    if (mSession->hasCurrentTimeSheet())
    {
        stopCurrentTimeSheet();
    }
    else
    {
        fillFromTimesheet(timeSheet);
        startPendingTimeSheet();
    }
}

void ActivityWidget::onHistoryTimeSheetFillRequested(const TimeSheet& timeSheet)
{
    if (mSession->hasCurrentTimeSheet())
    {
        stopCurrentTimeSheet();
    }
    fillFromTimesheet(timeSheet);
}

void ActivityWidget::fillFromTimesheet(const TimeSheet& timeSheet)
{
    updateProjectsCombo();
    mUi->cbProject->setCurrentIndex(mUi->cbProject->findData(timeSheet.project.id));
    mUi->cbActivity->setCurrentIndex(mUi->cbActivity->findData(timeSheet.activity.id));
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

        timeSheet.project.id  = mUi->cbProject->currentData().toInt();
        timeSheet.activity.id = mUi->cbActivity->currentData().toInt();

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

    mUi->cbProject->setEnabled(noTimeSheetRunning);
    mUi->cbActivity->setEnabled(noTimeSheetRunning && !mUi->cbProject->currentText().isEmpty());

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
        if (mSession->hasCurrentTimeSheet()) {}
    }
}

void ActivityWidget::updateProjectsCombo()
{
    if (mSession)
    {
        // When no customer selected, list all projects

        if (mSession->hasCurrentTimeSheet())
        {
            auto projectIndex = mUi->cbProject->findData(mSession->currentTimeSheet()->project.id);
            if (projectIndex >= 0)
            {
                mUi->cbProject->setCurrentIndex(projectIndex);
            }
            else
            {
                spdlog::error("Cannot find '{}' project", mSession->currentTimeSheet()->project.name);
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
                spdlog::error("Cannot find '{}' activity", mSession->currentTimeSheet()->activity.name);
            }
        }
        else if (!projectId.has_value())
        {
            mUi->cbActivity->setCurrentText("");
        }
    }
}

void ActivityWidget::updateRecentTimeSheetsView()
{
    mUi->lwHistory->clear();
    for (const auto& timeSheet : mSession->cache().recentTimeSheets())
    {
        auto timeSheetListWidgetItem = new TimeSheetListWidgetItem(timeSheet);

        auto item = new QListWidgetItem;
        item->setSizeHint(timeSheetListWidgetItem->sizeHint());

        mUi->lwHistory->addItem(item);
        mUi->lwHistory->setItemWidget(item, timeSheetListWidgetItem);

        connect(timeSheetListWidgetItem, &TimeSheetListWidgetItem::timeSheetStartRequested, this, &ActivityWidget::onHistoryTimeSheetStartRequested);
        connect(timeSheetListWidgetItem, &TimeSheetListWidgetItem::timeSheetFillRequested, this, &ActivityWidget::onHistoryTimeSheetFillRequested);
    }
}

void ActivityWidget::startPendingTimeSheet()
{
    if (mPendingStartRequest.has_value())
    {
        TimeSheet timeSheet;

        timeSheet.beginAt     = mSession->computeTZDateTime(QDateTime::currentDateTime());
        timeSheet.project.id  = mPendingStartRequest->project.id;
        timeSheet.activity.id = mPendingStartRequest->activity.id;

        mPendingStartRequest.reset();

        auto timeSheetResult = mSession->client()->startTimeSheet(timeSheet, mSession->timeSheetConfig().trackingMode);

        connect(timeSheetResult, &KimaiApiBaseResult::ready, this, [this, timeSheetResult] {
            mSession->refreshCurrentTimeSheet();
            timeSheetResult->deleteLater();
        });
        connect(timeSheetResult, &KimaiApiBaseResult::error, [timeSheetResult]() { timeSheetResult->deleteLater(); });
    }
}
