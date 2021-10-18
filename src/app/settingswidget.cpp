#include "settingswidget.h"
#include "ui_settingswidget.h"

#include "client/kimairequestfactory.h"
#include "settings.h"

#include <QAction>
#include <QDir>
#include <QMessageBox>
#include <QTranslator>

using namespace kemai::app;
using namespace kemai::client;
using namespace kemai::core;

SettingsWidget::SettingsWidget(QWidget* parent) : QWidget(parent), mUi(new Ui::SettingsWidget), mKimaiClient(new KimaiClient)
{
    mUi->setupUi(this);

    auto addLanguage = [cbLanguage = mUi->cbLanguage](const QString& language) {
        QLocale locale(language);
        cbLanguage->addItem(QString("%1 [%2]").arg(QLocale::languageToString(locale.language()), QLocale::countryToString(locale.country())), locale);
    };

    mActToggleTokenVisible = mUi->leToken->addAction(QIcon(":/icons/visible-off"), QLineEdit::TrailingPosition);

    // Add default en_US language
    addLanguage("en_US");

    // Add available translations
    QDir l10nDir(":/l10n");
    for (const auto& fLang : l10nDir.entryList())
    {
        QTranslator translator;
        if (translator.load(l10nDir.absoluteFilePath(fLang), "kemai", "_", ":/l10n"))
        {
            addLanguage(translator.language());
        }
    }

    connect(mKimaiClient.data(), &KimaiClient::requestError, this, &SettingsWidget::onClientError);
    connect(mKimaiClient.data(), &KimaiClient::replyReceived, this, &SettingsWidget::onClientReply);
    connect(mUi->btTest, &QPushButton::clicked, this, &SettingsWidget::onBtTestClicked);
    connect(mUi->btCancel, &QPushButton::clicked, this, &SettingsWidget::onBtCancelClicked);
    connect(mUi->btSave, &QPushButton::clicked, this, &SettingsWidget::onBtSaveClicked);
    connect(mActToggleTokenVisible, &QAction::triggered, [&]() {
        if (mUi->leToken->echoMode() == QLineEdit::Password)
        {
            mUi->leToken->setEchoMode(QLineEdit::Normal);
            mActToggleTokenVisible->setIcon(QIcon(":/icons/visible"));
        }
        else
        {
            mUi->leToken->setEchoMode(QLineEdit::Password);
            mActToggleTokenVisible->setIcon(QIcon(":/icons/visible-off"));
        }
    });

    // show dialog if language changes from settings
    connect(mUi->cbLanguage, &QComboBox::currentTextChanged, [&](const QString&) {
        auto settings = Settings::load();
        if (settings.kemai.language != mUi->cbLanguage->currentData().toLocale())
        {
            QMessageBox::warning(this, tr(""), tr("Language changed. Application restart is required."));
        }
    });

    loadSettings();
}

SettingsWidget::~SettingsWidget()
{
    delete mUi;
}

void SettingsWidget::onClientError(const QString& errorMsg)
{
    mUi->lbTestResult->setText(errorMsg);
}

void SettingsWidget::onClientReply(const KimaiReply& reply)
{
    if (!reply.isValid())
    {
        mUi->lbTestResult->setText(tr("Invalid reply."));
    }
    else
    {
        auto version = reply.get<KimaiVersion>();
        mUi->lbTestResult->setText(tr("Connected to Kimai %1").arg(version.kimai.toString()));
    }
}

void SettingsWidget::loadSettings()
{
    auto settings = Settings::load();
    mUi->leHost->setText(settings.kimai.host);
    mUi->leUsername->setText(settings.kimai.username);
    mUi->leToken->setText(settings.kimai.token);
    mUi->cbCloseToSystemTray->setChecked(settings.kemai.closeToSystemTray);

    auto idLanguage = mUi->cbLanguage->findData(settings.kemai.language);
    if (idLanguage >= 0)
    {
        mUi->cbLanguage->setCurrentIndex(idLanguage);
    }
}

void SettingsWidget::saveSettings()
{
    Settings settings;
    settings.kimai.host              = mUi->leHost->text();
    settings.kimai.username          = mUi->leUsername->text();
    settings.kimai.token             = mUi->leToken->text();
    settings.kemai.closeToSystemTray = mUi->cbCloseToSystemTray->isChecked();
    settings.kemai.language          = mUi->cbLanguage->currentData().toLocale();
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
    emit cancelled();
}

void SettingsWidget::onBtSaveClicked()
{
    saveSettings();
    emit settingsSaved();
}
