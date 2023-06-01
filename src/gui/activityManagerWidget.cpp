#include "activityManagerWidget.h"
#include "ui_activityManagerWidget.h"

#include <QPushButton>

using namespace kemai;

ActivityManagerWidget::ActivityManagerWidget(QWidget* parent) : QWidget(parent), mUi(new Ui::ActivityManagerWidget), mAddActivityWidgetButton(new QToolButton)
{
    mUi->setupUi(this);

    mAddActivityWidgetButton->setText("");
    mAddActivityWidgetButton->setIcon(QIcon(":/icons/add"));
    mUi->tabWidget->setCornerWidget(mAddActivityWidgetButton);

    connect(mAddActivityWidgetButton, &QToolButton::clicked, this, &ActivityManagerWidget::onAddActivityWidgetButtonClicked);
}

ActivityManagerWidget::~ActivityManagerWidget()
{
    delete mUi;
}

void ActivityManagerWidget::setKemaiSession(std::shared_ptr<KemaiSession> kemaiSession)
{
    mSession = std::move(kemaiSession);

    if (mSession)
    {
        connect(mSession.get(), &KemaiSession::currentTimeSheetsChanged, this, &ActivityManagerWidget::onSessionCurrentTimeSheetsChanged);
        connect(&mSession->cache(), &KimaiCache::synchronizeStarted, this, [this]() { setEnabled(false); });
        connect(&mSession->cache(), &KimaiCache::synchronizeFinished, this, &ActivityManagerWidget::onSessionCacheSynchronizeFinished);
    }
    else
    {
        setEnabled(false);
    }
}

void ActivityManagerWidget::onSessionCurrentTimeSheetsChanged()
{
    auto timeSheets = mSession->currentTimeSheets();
    for (const auto& timeSheet : timeSheets)
    {
        auto index = findOrCreateTabForTimeSheet(timeSheet);
        updateTimeSheetTab(index, timeSheet);
    }
}

void ActivityManagerWidget::onAddActivityWidgetButtonClicked() {}

void ActivityManagerWidget::onSessionCacheSynchronizeFinished()
{
    setEnabled(true);
}

std::optional<int> ActivityManagerWidget::findTabIndexForTimeSheet(const TimeSheet& timeSheet) const
{
    for (int index = 0; index < mUi->tabWidget->count(); ++index)
    {
        if (mUi->tabWidget->widget(index)->objectName() == QString("timeSheetWidget_%1").arg(timeSheet.id))
        {
            return index;
        }
    }
    return {};
}

int ActivityManagerWidget::findOrCreateTabForTimeSheet(const TimeSheet& timeSheet)
{
    auto index = findTabIndexForTimeSheet(timeSheet);
    if (index.has_value())
    {
        return index.value();
    }
    auto widget = new QPushButton;
    widget->setObjectName(QString("timeSheetWidget_%1").arg(timeSheet.id));
    return mUi->tabWidget->addTab(widget, widget->objectName());
}

void ActivityManagerWidget::updateTimeSheetTab(int index, const TimeSheet& timeSheet)
{
    mUi->tabWidget->setTabToolTip(index, QString("%1/%2/%3").arg(timeSheet.activity.name, timeSheet.project.name, ""));
}
