#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include "client/kimairequestfactory.h"
#include "settings.h"

#include <QAction>
#include <QDir>
#include <QMessageBox>
#include <QTranslator>

using namespace kemai::app;
using namespace kemai::client;
using namespace kemai::core;

/*
 * Static helper
 */
enum EProfileItemRole
{
    PIR_Host = Qt::UserRole + 1,
    PIR_User,
    PIR_Token
};

/*
 *
 */
SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent), mUi(new Ui::SettingsDialog), mKimaiClient(new KimaiClient)
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

    connect(mKimaiClient.data(), &KimaiClient::requestError, this, &SettingsDialog::onClientError);
    connect(mKimaiClient.data(), &KimaiClient::replyReceived, this, &SettingsDialog::onClientReply);
    connect(mUi->testButton, &QPushButton::clicked, this, &SettingsDialog::onBtTestClicked);

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

    connect(mUi->profilesListWidget, &QListWidget::currentItemChanged, this, &SettingsDialog::onProfilesListCurrentItemChanged);
    connect(mUi->leName, &QLineEdit::textChanged, this, &SettingsDialog::onProfileFieldValueChanged);
    connect(mUi->leHost, &QLineEdit::textChanged, this, &SettingsDialog::onProfileFieldValueChanged);
    connect(mUi->leUsername, &QLineEdit::textChanged, this, &SettingsDialog::onProfileFieldValueChanged);
    connect(mUi->leToken, &QLineEdit::textChanged, this, &SettingsDialog::onProfileFieldValueChanged);
    connect(mUi->addProfileButton, &QToolButton::clicked, this, &SettingsDialog::onProfileAddButtonClicked);
    connect(mUi->delProfileButton, &QToolButton::clicked, this, &SettingsDialog::onProfileDelButtonClicked);

    // show dialog if language changes from settings
    connect(mUi->cbLanguage, &QComboBox::currentTextChanged, [&](const QString&) {
        auto settings = Settings::load();
        if (settings.kemai.language != mUi->cbLanguage->currentData().toLocale())
        {
            QMessageBox::warning(this, tr(""), tr("Language changed. Application restart is required."));
        }
    });
}

SettingsDialog::~SettingsDialog() = default;

void SettingsDialog::setSettings(const Settings& settings)
{
    m_settings = settings;

    /*
     * General settings
     */
    mUi->cbCloseToSystemTray->setChecked(m_settings.kemai.closeToSystemTray);
    mUi->cbMinimizeToSystemTray->setChecked(m_settings.kemai.minimizeToSystemTray);

    auto idLanguage = mUi->cbLanguage->findData(m_settings.kemai.language);
    if (idLanguage >= 0)
    {
        mUi->cbLanguage->setCurrentIndex(idLanguage);
    }

    /*
     * Profiles
     */
    mUi->profilesListWidget->clear();
    for (const auto& profile : m_settings.profiles)
    {
        auto item = new QListWidgetItem;
        profileToItemList(profile, item);
        mUi->profilesListWidget->addItem(item);
    }
}

Settings SettingsDialog::settings() const
{
    auto settings = m_settings;

    settings.kemai.closeToSystemTray    = mUi->cbCloseToSystemTray->isChecked();
    settings.kemai.minimizeToSystemTray = mUi->cbMinimizeToSystemTray->isChecked();
    settings.kemai.language             = mUi->cbLanguage->currentData().toLocale();

    settings.profiles.clear();
    for (int i = 0; i < mUi->profilesListWidget->count(); ++i)
    {
        auto profile = profileFromItemList(mUi->profilesListWidget->item(i));
        if (!profile.name.isEmpty())
        {
            settings.profiles << profile;
        }
    }

    return settings;
}

void SettingsDialog::onProfilesListCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* /*previous*/)
{
    auto profile = profileFromItemList(current);
    mUi->leName->setText(profile.name);
    mUi->leHost->setText(profile.host);
    mUi->leUsername->setText(profile.username);
    mUi->leToken->setText(profile.token);

    mUi->delProfileButton->setEnabled(current != nullptr);
}

void SettingsDialog::onBtTestClicked()
{
    mUi->testResultLabel->clear();

    mKimaiClient->setHost(mUi->leHost->text());
    mKimaiClient->setUsername(mUi->leUsername->text());
    mKimaiClient->setToken(mUi->leToken->text());
    mKimaiClient->sendRequest(KimaiRequestFactory::version());
}

void SettingsDialog::onClientError(const QString& errorMsg)
{
    mUi->testResultLabel->setText(errorMsg);
}

void SettingsDialog::onClientReply(const KimaiReply& reply)
{
    if (!reply.isValid())
    {
        mUi->testResultLabel->setText(tr("Invalid reply."));
    }
    else
    {
        auto version = reply.get<KimaiVersion>();
        mUi->testResultLabel->setText(tr("Connected to Kimai %1").arg(version.kimai.toString()));
    }
}

void SettingsDialog::onProfileFieldValueChanged()
{
    auto item = mUi->profilesListWidget->currentItem();
    Settings::Profile profile;
    profile.name     = mUi->leName->text();
    profile.host     = mUi->leHost->text();
    profile.username = mUi->leUsername->text();
    profile.token    = mUi->leToken->text();

    profileToItemList(profile, item);
}

void SettingsDialog::onProfileAddButtonClicked()
{
    Settings::Profile profile;
    profile.name = "Default";

    auto item = new QListWidgetItem;
    profileToItemList(profile, item);
    mUi->profilesListWidget->addItem(item);

    mUi->profilesListWidget->setCurrentItem(item);
}

void SettingsDialog::onProfileDelButtonClicked()
{
    delete mUi->profilesListWidget->takeItem(mUi->profilesListWidget->currentRow());
}

Settings::Profile SettingsDialog::profileFromItemList(QListWidgetItem* item) const
{
    if (item == nullptr)
    {
        return {};
    }

    Settings::Profile profile;
    profile.name     = item->text();
    profile.host     = item->data(PIR_Host).toString();
    profile.username = item->data(PIR_User).toString();
    profile.token    = item->data(PIR_Token).toString();
    return profile;
}

void SettingsDialog::profileToItemList(const Settings::Profile& profile, QListWidgetItem* item)
{
    if (item == nullptr)
    {
        return;
    }

    item->setText(profile.name);
    item->setData(PIR_Host, profile.host);
    item->setData(PIR_User, profile.username);
    item->setData(PIR_Token, profile.token);
}
