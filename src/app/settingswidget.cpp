#include "settingswidget.h"
#include "ui_settingswidget.h"

#include "kemai/kimairequestfactory.h"
#include "kemai/settings.h"

#include <QAction>
#include <QStackedWidget>

using namespace kemai::app;
using namespace kemai::client;
using namespace kemai::core;

SettingsWidget::SettingsWidget(QWidget* parent)
    : QWidget(parent), mUi(new Ui::SettingsWidget), mKimaiClient(new KimaiClient)
{
    mUi->setupUi(this);
    loadSettings();

    connect(mKimaiClient.data(), &KimaiClient::requestError, this, &SettingsWidget::onClientError);
    connect(mKimaiClient.data(), &KimaiClient::replyReceived, this, &SettingsWidget::onClientReply);
    connect(mUi->btTest, &QPushButton::clicked, this, &SettingsWidget::onBtTestClicked);
    connect(mUi->btCancel, &QPushButton::clicked, this, &SettingsWidget::onBtCancelClicked);
    connect(mUi->btSave, &QPushButton::clicked, this, &SettingsWidget::onBtSaveClicked);

    auto actToggleTokenVisible = mUi->leToken->addAction(QIcon(":/icons/visible"), QLineEdit::TrailingPosition);
    connect(actToggleTokenVisible, &QAction::triggered, [&]() {
        if (mUi->leToken->echoMode() == QLineEdit::Password)
            mUi->leToken->setEchoMode(QLineEdit::Normal);
        else
            mUi->leToken->setEchoMode(QLineEdit::Password);
    });
}

SettingsWidget::~SettingsWidget()
{
    delete mUi;
}

void SettingsWidget::setActivityWidgetIndex(int idx)
{
    mActivityWidgetIndex = idx;
}

void SettingsWidget::backToActivity()
{
    if (auto stackedWidget = qobject_cast<QStackedWidget*>(parentWidget()))
    {
        stackedWidget->setCurrentIndex(mActivityWidgetIndex);
    }
}

void SettingsWidget::onClientError(const QString& errorMsg)
{
    mUi->lbTestResult->setText(errorMsg);
}

void SettingsWidget::onClientReply(const KimaiReply& reply)
{
    if (not reply.isValid())
    {
        mUi->lbTestResult->setText(tr("Invalid reply."));
    }
    else
    {
        auto version = reply.get<KimaiVersion>();
        mUi->lbTestResult->setText(tr("Connected to Kimai %1").arg(version.kimai));
    }
}

void SettingsWidget::loadSettings()
{
    auto settings = Settings::load();
    mUi->leHost->setText(settings.kimai.host);
    mUi->leUsername->setText(settings.kimai.username);
    mUi->leToken->setText(settings.kimai.token);
    mUi->cbCloseToSystemTray->setChecked(settings.kemai.closeToSystemTray);
}

void SettingsWidget::saveSettings()
{
    Settings settings;
    settings.kimai.host              = mUi->leHost->text();
    settings.kimai.username          = mUi->leUsername->text();
    settings.kimai.token             = mUi->leToken->text();
    settings.kemai.closeToSystemTray = mUi->cbCloseToSystemTray->isChecked();
    Settings::save(settings);
}

void SettingsWidget::onBtTestClicked()
{
    mUi->lbTestResult->clear();

    mKimaiClient->setHost(mUi->leHost->text());
    mKimaiClient->setUsername(mUi->leUsername->text());
    mKimaiClient->setToken(mUi->leToken->text());
    mKimaiClient->sendRequest(KimaiRequestFactory::version());
}

void SettingsWidget::onBtCancelClicked()
{
    loadSettings();
    backToActivity();
}

void SettingsWidget::onBtSaveClicked()
{
    saveSettings();
    backToActivity();
}
