#include "simpleActivityWidget.h"
#include "ui_simpleActivityWidget.h"

#include "timeSheetListWidgetItem.h"

using namespace kemai;

SimpleActivityWidget::SimpleActivityWidget(QWidget* parent) : QWidget(parent), mUi(std::make_unique<Ui::SimpleActivityWidget>())
{
    mUi->setupUi(this);

    mUi->scrollAreaWidgetContents->setLayout(new QVBoxLayout);
    mUi->scrollAreaWidgetContents->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    //    connect(mUi->cbProject, &QComboBox::currentTextChanged, this, &SimpleActivityWidget::onCbProjectFieldChanged);
    connect(mUi->cbProject, &QComboBox::currentIndexChanged, this, &SimpleActivityWidget::onCbProjectFieldChanged);
    //    connect(mUi->cbActivity, &QComboBox::currentTextChanged, this, &SimpleActivityWidget::onCbActivityFieldChanged);
    connect(mUi->cbActivity, &QComboBox::currentIndexChanged, this, &SimpleActivityWidget::onCbActivityFieldChanged);
    connect(mUi->btCreate, &QPushButton::clicked, this, &SimpleActivityWidget::onBtCreateClicked);
    connect(&mSecondTimer, &QTimer::timeout, this, &SimpleActivityWidget::onSecondTimeout);

    mUi->cbProject->setModel(&mCustomerProjectModel);

    mSecondTimer.setInterval(std::chrono::seconds(1));
    mSecondTimer.setTimerType(Qt::PreciseTimer);
    mSecondTimer.start();
}

SimpleActivityWidget::~SimpleActivityWidget() = default;

void SimpleActivityWidget::setKemaiSession(std::shared_ptr<KemaiSession> kemaiSession)
{
    mSession = std::move(kemaiSession);

    mUi->cbActivity->clear();
    mUi->cbProject->clear();

    if (mSession)
    {
        connect(mSession.get(), &KemaiSession::currentTimeSheetsChanged, this, &SimpleActivityWidget::updateTimeSheets);
        connect(&mSession->cache(), &KimaiCache::synchronizeStarted, this, [this]() { setEnabled(false); });
        connect(&mSession->cache(), &KimaiCache::synchronizeFinished, this, &SimpleActivityWidget::onSessionCacheSynchronizeFinished);
    }
    else
    {
        setEnabled(false);
    }
}

void SimpleActivityWidget::onCbProjectFieldChanged()
{
    if (mSession)
    {
        auto projectId = mUi->cbProject->currentText().isEmpty() ? std::nullopt : std::optional<int>(mUi->cbProject->currentData().toInt());
        mUi->cbActivity->setFilter(mSession->cache().activities(projectId));
    }
    updateControls();
}

void SimpleActivityWidget::onCbActivityFieldChanged()
{
    updateControls();
}

void SimpleActivityWidget::onBtCreateClicked()
{
    if (mSession)
    {
        TimeSheet timeSheet;

        timeSheet.beginAt     = mSession->computeTZDateTime(QDateTime::currentDateTime());
        timeSheet.project.id  = mUi->cbProject->currentData().toInt();
        timeSheet.activity.id = mUi->cbActivity->currentData().toInt();

        mSession->client()->startTimeSheet(timeSheet, mSession->timeSheetConfig().trackingMode);
    }
}

void SimpleActivityWidget::onSessionCacheSynchronizeFinished()
{
    mCustomerProjectModel.setCustomersProjects(mSession->cache().customers(), mSession->cache().projects());
    mUi->cbActivity->setKimaiData(mSession->cache().activities());
    updateTimeSheets();

    setEnabled(true);
}

void SimpleActivityWidget::onTimeSheetStartRequested(const TimeSheet& timeSheet)
{
    auto timeSheetResult = mSession->client()->startTimeSheet(timeSheet, mSession->timeSheetConfig().trackingMode);

    connect(timeSheetResult, &KimaiApiBaseResult::ready, this, [this, timeSheetResult] {
        mSession->refreshCurrentTimeSheet();
        timeSheetResult->deleteLater();
    });
    connect(timeSheetResult, &KimaiApiBaseResult::error, [timeSheetResult]() { timeSheetResult->deleteLater(); });
}

void SimpleActivityWidget::onTimeSheetStopRequested(const TimeSheet& timeSheet)
{
    auto updatedTimeSheet  = timeSheet;
    updatedTimeSheet.endAt = mSession->computeTZDateTime(QDateTime::currentDateTime());

    auto timeSheetResult = mSession->client()->updateTimeSheet(updatedTimeSheet, mSession->timeSheetConfig().trackingMode);
    
    connect(timeSheetResult, &KimaiApiBaseResult::ready, this, [this, timeSheetResult] {
        mSession->refreshCurrentTimeSheet();
        timeSheetResult->deleteLater();
    });
    connect(timeSheetResult, &KimaiApiBaseResult::error, [timeSheetResult]() { timeSheetResult->deleteLater(); });
}

void SimpleActivityWidget::onSecondTimeout()
{
    for (auto* child : mUi->scrollAreaWidgetContents->findChildren<TimeSheetListWidgetItem*>())
    {
        child->updateDuration();
    }
}

void SimpleActivityWidget::updateControls()
{
    auto projectId  = mUi->cbProject->currentData().toInt();
    auto activityId = mUi->cbActivity->currentData().toInt();
    mUi->btCreate->setEnabled(projectId >= 0 && activityId >= 0 && mSession != nullptr);
}

void SimpleActivityWidget::updateTimeSheets()
{
    if (mSession)
    {
        /*
         * Clear previous items
         */
        for (auto* child : mUi->scrollAreaWidgetContents->findChildren<TimeSheetListWidgetItem*>())
        {
            mUi->scrollAreaWidgetContents->layout()->removeWidget(child);
            child->setParent(nullptr);
            child->deleteLater();
        }

        /*
         * Insert current running time sheets
         */
        for (const auto& timeSheet : mSession->currentTimeSheets())
        {
            addTimeSheet(timeSheet, true);
        }

        /*
         * Insert history
         */
        for (const auto& timeSheet : mSession->cache().recentTimeSheets())
        {
            addTimeSheet(timeSheet);
        }
    }
}

void SimpleActivityWidget::addTimeSheet(const TimeSheet& timeSheet, bool isActive)
{
    auto* widget = new TimeSheetListWidgetItem(timeSheet, mUi->scrollAreaWidgetContents);
    widget->setIsActive(isActive);

    // Push back widget before spacer item
    auto* scrollLayout = qobject_cast<QVBoxLayout*>(mUi->scrollAreaWidgetContents->layout());
    scrollLayout->insertWidget(scrollLayout->count() - 1, widget);

    //
    connect(widget, &TimeSheetListWidgetItem::timeSheetStartRequested, this, &SimpleActivityWidget::onTimeSheetStartRequested);
    connect(widget, &TimeSheetListWidgetItem::timeSheetStopRequested, this, &SimpleActivityWidget::onTimeSheetStopRequested);
}
