#include "activityManagerWidget.h"
#include "ui_activityManagerWidget.h"

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
        connect(&mSession->cache(), &KimaiCache::synchronizeStarted, this, [this]() { setEnabled(false); });
        connect(&mSession->cache(), &KimaiCache::synchronizeFinished, this, &ActivityManagerWidget::onSessionCacheSynchronizeFinished);
    }
    else
    {
        setEnabled(false);
    }
}

void ActivityManagerWidget::onAddActivityWidgetButtonClicked() {}

void ActivityManagerWidget::onSessionCacheSynchronizeFinished()
{
    setEnabled(true);
}
