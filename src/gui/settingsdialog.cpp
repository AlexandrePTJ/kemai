#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QAction>
#include <QDir>
#include <QMessageBox>
#include <QTranslator>

#include "kemai_config.h"

using namespace kemai;

SettingsDialog::SettingsDialog(const std::shared_ptr<DesktopEventsMonitor>& desktopEventsMonitor, QWidget* parent)
    : QDialog(parent), mUi(new Ui::SettingsDialog), mKimaiClient(std::make_unique<KimaiClient>())
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

    // Enable events depending on platform
    if (desktopEventsMonitor)
    {
        mUi->cbStopOnLock->setEnabled(desktopEventsMonitor->hasLockSupport());
        mUi->cbStopOnLock->setVisible(false);
        mUi->cbStopOnIdle->setEnabled(desktopEventsMonitor->hasIdleSupport());
    }

    // Disable check update checkbox when disabled at build-time
#ifndef KEMAI_ENABLE_UPDATE_CHECK
    mUi->cbCheckUpdateAtStartup->setChecked(false);
    mUi->cbCheckUpdateAtStartup->setEnabled(false);
#endif // KEMAI_ENABLE_UPDATE_CHECK
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
#ifdef KEMAI_ENABLE_UPDATE_CHECK
    mUi->cbCheckUpdateAtStartup->setChecked(m_settings.kemai.checkUpdateAtStartup);
#else
    mUi->cbCheckUpdateAtStartup->setChecked(false);
#endif // KEMAI_ENABLE_UPDATE_CHECK

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
        item->setText(profile.name);
        item->setData(Qt::UserRole, profile.id);
        mUi->profilesListWidget->addItem(item);
    }

    /*
     * Events
     */
    mUi->cbStopOnLock->setChecked(m_settings.events.stopOnLock);
    mUi->cbStopOnIdle->setChecked(m_settings.events.stopOnIdle);
    mUi->sbIdleDelay->setValue(m_settings.events.idleDelayMinutes);
}

Settings SettingsDialog::settings() const
{
    auto settings = m_settings;

    settings.kemai.closeToSystemTray    = mUi->cbCloseToSystemTray->isChecked();
    settings.kemai.minimizeToSystemTray = mUi->cbMinimizeToSystemTray->isChecked();
    settings.kemai.checkUpdateAtStartup = mUi->cbCheckUpdateAtStartup->isChecked();
    settings.kemai.language             = mUi->cbLanguage->currentData().toLocale();

    settings.events.stopOnLock       = mUi->cbStopOnLock->isChecked();
    settings.events.stopOnIdle       = mUi->cbStopOnIdle->isChecked();
    settings.events.idleDelayMinutes = mUi->sbIdleDelay->value();

    // Profiles are directly managed from buttons on fields updates.

    return settings;
}

void SettingsDialog::onProfilesListCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* /*previous*/)
{
    QSignalBlocker nameSignalBlocker(mUi->leName);
    QSignalBlocker hostSignalBlocker(mUi->leHost);
    QSignalBlocker usernameSignalBlocker(mUi->leUsername);
    QSignalBlocker tokenSignalBlocker(mUi->leToken);

    Settings::Profile profile;

    if (current != nullptr)
    {
        auto profileIt = m_settings.findProfileRef(current->data(Qt::UserRole).toUuid());
        if (profileIt != m_settings.profiles.end())
        {
            profile = *profileIt;
        }
    }

    auto hasProfile = !profile.id.isNull();
    mUi->leName->setText(profile.name);
    mUi->leHost->setText(profile.host);
    mUi->leUsername->setText(profile.username);
    mUi->leToken->setText(profile.token);

    mUi->delProfileButton->setEnabled(hasProfile);
    mUi->leName->setEnabled(hasProfile);
    mUi->leHost->setEnabled(hasProfile);
    mUi->leUsername->setEnabled(hasProfile);
    mUi->leToken->setEnabled(hasProfile);
    mUi->testButton->setEnabled(hasProfile);
}

void SettingsDialog::onBtTestClicked()
{
    mUi->testResultLabel->clear();

    mKimaiClient->setHost(mUi->leHost->text());
    mKimaiClient->setUsername(mUi->leUsername->text());
    mKimaiClient->setToken(mUi->leToken->text());

    auto versionResult = mKimaiClient->requestKimaiVersion();
    connect(versionResult, &KimaiApiBaseResult::ready, this, [this, versionResult]() {
        mUi->testResultLabel->setText(tr("Connected to Kimai %1").arg(versionResult->getResult().kimai.toString()));
        versionResult->deleteLater();
    });
    connect(versionResult, &KimaiApiBaseResult::error, this, [this, versionResult]() {
        mUi->testResultLabel->setText(versionResult->errorMessage());
        versionResult->deleteLater();
    });
}

void SettingsDialog::onProfileFieldValueChanged()
{
    auto item = mUi->profilesListWidget->currentItem();
    if (item != nullptr)
    {
        auto profile = m_settings.findProfileRef(item->data(Qt::UserRole).toUuid());
        if (profile != m_settings.profiles.end())
        {
            profile->name     = mUi->leName->text();
            profile->host     = mUi->leHost->text();
            profile->username = mUi->leUsername->text();
            profile->token    = mUi->leToken->text();

            item->setText(profile->name);
        }
    }
}

void SettingsDialog::onProfileAddButtonClicked()
{
    Settings::Profile profile;
    profile.id   = QUuid::createUuid();
    profile.name = "Default";

    m_settings.profiles.push_back(profile);

    auto item = new QListWidgetItem;
    item->setText(profile.name);
    item->setData(Qt::UserRole, profile.id);
    mUi->profilesListWidget->addItem(item);

    mUi->profilesListWidget->setCurrentItem(item);
}

void SettingsDialog::onProfileDelButtonClicked()
{
    auto item = mUi->profilesListWidget->takeItem(mUi->profilesListWidget->currentRow());
    if (item != nullptr)
    {
        auto it = std::find_if(m_settings.profiles.begin(), m_settings.profiles.end(),
                               [profileId = item->data(Qt::UserRole).toUuid()](const Settings::Profile& profile) { return profile.id == profileId; });
        if (it != m_settings.profiles.end())
        {
            auto pos = std::distance(m_settings.profiles.begin(), it);
            m_settings.profiles.removeAt(pos);
        }

        delete item;
    }
}
