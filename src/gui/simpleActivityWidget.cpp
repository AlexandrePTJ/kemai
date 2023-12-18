#include "simpleActivityWidget.h"
#include "ui_simpleActivityWidget.h"

using namespace kemai;

SimpleActivityWidget::SimpleActivityWidget(QWidget* parent) : QWidget(parent), mUi(std::make_unique<Ui::SimpleActivityWidget>())
{
    mUi->setupUi(this);

    connect(mUi->cbProject, &QComboBox::currentTextChanged, this, &SimpleActivityWidget::onCbProjectFieldChanged);
    connect(mUi->cbProject, &QComboBox::currentIndexChanged, this, &SimpleActivityWidget::onCbProjectFieldChanged);
    connect(mUi->cbActivity, &QComboBox::currentTextChanged, this, &SimpleActivityWidget::onCbActivityFieldChanged);
    connect(mUi->cbActivity, &QComboBox::currentIndexChanged, this, &SimpleActivityWidget::onCbActivityFieldChanged);

    mUi->cbProject->setModel(&mCustomerProjectModel);
}

SimpleActivityWidget::~SimpleActivityWidget() = default;

void SimpleActivityWidget::setKemaiSession(std::shared_ptr<KemaiSession> kemaiSession)
{
    mSession = std::move(kemaiSession);

    mUi->cbActivity->clear();
    mUi->cbProject->clear();

    if (mSession)
    {
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
    updateActivitiesCombo();
}

void SimpleActivityWidget::onCbActivityFieldChanged() {}

void SimpleActivityWidget::onBtCreateClicked() {}

void SimpleActivityWidget::onSessionCacheSynchronizeFinished()
{
    mCustomerProjectModel.setCustomersProjects(mSession->cache().customers(), mSession->cache().projects());

    setEnabled(true);
}

void SimpleActivityWidget::updateProjectsCombo() {}

void SimpleActivityWidget::updateActivitiesCombo() {}

void SimpleActivityWidget::updateTimeSheets() {}
