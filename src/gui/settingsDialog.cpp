#include "settingsDialog.h"
#include "ui_settingsDialog.h"

#include <QAction>
#include <QDir>
#include <QMessageBox>
#include <QTranslator>

#include "client/kimaiFeatures.h"
#include "kemaiConfig.h"

using namespace kemai;

/*
 * Static helpers
 */
static void toggleLineEditEchoMode(QLineEdit* lineEdit, QAction* action)
{
    if (lineEdit->echoMode() == QLineEdit::Password)
    {
        lineEdit->setEchoMode(QLineEdit::Normal);
        action->setIcon(QIcon(":/icons/visible"));
    }
    else
    {
        lineEdit->setEchoMode(QLineEdit::Password);
        action->setIcon(QIcon(":/icons/visible-off"));
    }
}

/*
 * Class impl
 */
SettingsDialog::SettingsDialog(const std::shared_ptr<DesktopEventsMonitor>& desktopEventsMonitor, QWidget* parent)
    : QDialog(parent), mUi(std::make_unique<Ui::SettingsDialog>()), mKimaiClient(std::make_unique<KimaiClient>())
{
    mUi->setupUi(this);

    toggleProfileAPITokenWarningVisibility(/*visible=*/false);

    auto addLanguage = [cbLanguage = mUi->cbLanguage](const QString& language) {
        QLocale locale(language);
        cbLanguage->addItem(QString("%1 [%2]").arg(QLocale::languageToString(locale.language()), QLocale::countryToString(locale.country())), locale);
    };

    mActToggleTokenVisible    = mUi->leToken->addAction(QIcon(":/icons/visible-off"), QLineEdit::TrailingPosition);
    mActToggleAPITokenVisible = mUi->leAPIToken->addAction(QIcon(":/icons/visible-off"), QLineEdit::TrailingPosition);

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

    connect(mActToggleTokenVisible, &QAction::triggered, [&]() { toggleLineEditEchoMode(mUi->leToken, mActToggleTokenVisible); });
    connect(mActToggleAPITokenVisible, &QAction::triggered, [&]() { toggleLineEditEchoMode(mUi->leAPIToken, mActToggleAPITokenVisible); });

    connect(mUi->profilesListWidget, &QListWidget::currentItemChanged, this, &SettingsDialog::onProfilesListCurrentItemChanged);
    connect(mUi->leName, &QLineEdit::textChanged, this, &SettingsDialog::onProfileFieldValueChanged);
    connect(mUi->leHost, &QLineEdit::textChanged, this, &SettingsDialog::onProfileFieldValueChanged);
    connect(mUi->leUsername, &QLineEdit::textChanged, this, &SettingsDialog::onProfileFieldValueChanged);
    connect(mUi->leToken, &QLineEdit::textChanged, this, &SettingsDialog::onProfileFieldValueChanged);
    connect(mUi->leAPIToken, &QLineEdit::textChanged, this, &SettingsDialog::onProfileFieldValueChanged);
    connect(mUi->addProfileButton, &QToolButton::clicked, this, &SettingsDialog::onProfileAddButtonClicked);
    connect(mUi->delProfileButton, &QToolButton::clicked, this, &SettingsDialog::onProfileDelButtonClicked);

    // show dialog if language changes from settings
    connect(mUi->cbLanguage, &QComboBox::currentTextChanged, [&](const QString&) {
        const auto& settings = SettingsHelper::load();
        if (settings.kemai.language != mUi->cbLanguage->currentData().toLocale())
        {
            QMessageBox::warning(this, tr(""), tr("Language changed. Application restart is required."));
        }
    });

    // Enable events depending on platform
    if (desktopEventsMonitor)
    {
        mUi->cbStopOnLock->setEnabled(desktopEventsMonitor->hasLockSupport());
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
    mUi->cbAutoRefreshCurrentTimeSheet->setChecked(m_settings.events.autoRefreshCurrentTimeSheet);
    mUi->sbAutoRefreshDelay->setValue(m_settings.events.autoRefreshCurrentTimeSheetDelaySeconds);
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

    settings.events.autoRefreshCurrentTimeSheet             = mUi->cbAutoRefreshCurrentTimeSheet->isChecked();
    settings.events.autoRefreshCurrentTimeSheetDelaySeconds = mUi->sbAutoRefreshDelay->value();

    // Profiles are directly managed from buttons on fields updates.

    return settings;
}

void SettingsDialog::onProfilesListCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* /*previous*/)
{
    QSignalBlocker nameSignalBlocker(mUi->leName);
    QSignalBlocker hostSignalBlocker(mUi->leHost);
    QSignalBlocker usernameSignalBlocker(mUi->leUsername);
    QSignalBlocker tokenSignalBlocker(mUi->leToken);
    QSignalBlocker apiTokenSignalBlocker(mUi->leAPIToken);

    mUi->leName->clear();
    mUi->leHost->clear();
    mUi->leUsername->clear();
    mUi->leToken->clear();
    mUi->leAPIToken->clear();

    auto hasProfile = false;

    if (current != nullptr)
    {
        auto profileIt = std::find_if(m_settings.profiles.begin(), m_settings.profiles.end(),
                                      [profileId = current->data(Qt::UserRole).toUuid()](const auto& profile) { return profile.id == profileId; });

        if (profileIt != m_settings.profiles.end())
        {
            mUi->leName->setText(profileIt->name);
            mUi->leHost->setText(profileIt->host);
            mUi->leUsername->setText(profileIt->username);
            mUi->leToken->setText(profileIt->token);
            mUi->leAPIToken->setText(profileIt->apiToken);

            hasProfile = true;
        }
    }

    mUi->delProfileButton->setEnabled(hasProfile);
    mUi->leName->setEnabled(hasProfile);
    mUi->leHost->setEnabled(hasProfile);
    mUi->leUsername->setEnabled(hasProfile);
    mUi->leToken->setEnabled(hasProfile);
    mUi->leAPIToken->setEnabled(hasProfile);
    mUi->testButton->setEnabled(hasProfile);
}

void SettingsDialog::onBtTestClicked()
{
    mUi->testResultLabel->clear();
    toggleProfileAPITokenWarningVisibility(/*visible=*/false);

    mKimaiClient->setHost(mUi->leHost->text());
    mKimaiClient->setLegacyAuth(mUi->leUsername->text(), mUi->leToken->text());
    mKimaiClient->setAPIToken(mUi->leAPIToken->text());

    auto versionResult = mKimaiClient->requestKimaiVersion();
    connect(versionResult, &KimaiApiBaseResult::ready, this, [this, versionResult]() {
        auto kimaiVersion = versionResult->takeResult();

        mUi->testResultLabel->setText(tr("Connected to Kimai %1").arg(kimaiVersion.kimai.toString()));
        if (KimaiFeatures::shouldUseAPIToken(kimaiVersion.kimai) && mKimaiClient->isUsingLegacyAuth())
        {
            toggleProfileAPITokenWarningVisibility(/*visible=*/true);
        }
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
        const auto profileId = item->data(Qt::UserRole).toUuid();
        auto profileIt =
            std::find_if(m_settings.profiles.begin(), m_settings.profiles.end(), [&profileId](const auto& profile) { return profile.id == profileId; });

        if (profileIt != m_settings.profiles.end())
        {
            profileIt->name     = mUi->leName->text();
            profileIt->host     = mUi->leHost->text();
            profileIt->username = mUi->leUsername->text();
            profileIt->token    = mUi->leToken->text();
            profileIt->apiToken = mUi->leAPIToken->text();

            item->setText(profileIt->name);
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
        //        auto it = std::find_if(m_settings.profiles.begin(), m_settings.profiles.end(),
        //                               [profileId = item->data(Qt::UserRole).toUuid()](const Settings::Profile& profile) { return profile.id == profileId; });
        //        if (it != m_settings.profiles.end())
        //        {
        //            auto pos = std::distance(m_settings.profiles.begin(), it);
        //            m_settings.profiles.removeAt(pos);
        //        }

        std::erase_if(m_settings.profiles,
                      [profileId = item->data(Qt::UserRole).toUuid()](const Settings::Profile& profile) { return profile.id == profileId; });

        delete item;
    }
}

void SettingsDialog::toggleProfileAPITokenWarningVisibility(bool visible)
{
    mUi->lbAPITokenWarningIcon->setVisible(visible);
    mUi->lbAPITokenWarningLabel->setVisible(visible);
}
