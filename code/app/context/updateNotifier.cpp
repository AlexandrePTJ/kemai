// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "updateNotifier.h"

// Qt headers
#include <QDateTime>

// 3rd party headers
#include <spdlog/spdlog.h>

// Project headers
#include "kemaiConfig.h"
#include <misc/customFmt.h>

namespace kemai
{
    UpdateNotifier::UpdateNotifier(QObject *parent):
    QObject(parent),
    m_checker(std::make_unique<KemaiUpdateChecker>())
    {
        // A local server instead of GitHub, for manual tests of the update strip
        if (const auto url = qEnvironmentVariable("KEMAI_UPDATE_URL"); !url.isEmpty())
        {
            m_checker->setReleasesUrl(QUrl(url));
        }
    }

    UpdateNotifier::~UpdateNotifier() = default;

    bool UpdateNotifier::enabled() const
    {
#ifdef KEMAI_ENABLE_UPDATE_CHECK
        return true;
#else
        return false;
#endif
    }

    bool UpdateNotifier::devBuild() const
    {
        return channel() == "dev";
    }

    QString UpdateNotifier::currentVersion() const
    {
        return KEMAI_VERSION;
    }

    QString UpdateNotifier::channel() const
    {
        return KEMAI_CHANNEL;
    }

    AppPreferences *UpdateNotifier::preferences() const
    {
        return m_preferences.data();
    }

    void UpdateNotifier::setPreferences(AppPreferences *preferences)
    {
        if (m_preferences != preferences)
        {
            m_preferences = preferences;
            emit preferencesChanged();
        }
    }

    UpdateNotifier::Status UpdateNotifier::status() const
    {
        return m_status;
    }

    UpdateNotifier::Source UpdateNotifier::source() const
    {
        return m_source;
    }

    QString UpdateNotifier::latestVersion() const
    {
        return m_latest.version;
    }

    bool UpdateNotifier::latestPrerelease() const
    {
        return m_latest.prerelease;
    }

    QUrl UpdateNotifier::releaseUrl() const
    {
        return m_latest.url;
    }

    UpdateNotifier::Failure UpdateNotifier::failure() const
    {
        return m_failure;
    }

    void UpdateNotifier::checkAtStart()
    {
        if (!enabled() || devBuild() || m_preferences.isNull() || !m_preferences->checkUpdatesAtStart())
        {
            return;
        }
        run(Startup);
    }

    void UpdateNotifier::checkNow(Source source)
    {
        if (enabled())
        {
            run(source);
        }
    }

    void UpdateNotifier::reset()
    {
        ++m_generation;
        setState(Idle);
    }

    void UpdateNotifier::showInStrip()
    {
        if (m_source != Settings)
        {
            return;
        }
        if (m_status == Available)
        {
            m_source = Menu;
            emit stateChanged();
        }
        else if (m_status == Checking)
        {
            // Still running: its result is announced like one at start (a new version only)
            m_source = Startup;
            emit stateChanged();
        }
        else
        {
            reset();
        }
    }

    void UpdateNotifier::run(Source source)
    {
        const auto generation = ++m_generation;
        m_source              = source;
        // A check at start shows nothing until it finds a version
        setState(source == Startup ? Idle : Checking);

        // A manual check reports even an ignored version
        const auto ignored = source == Startup && !m_preferences.isNull() ? m_preferences->ignoredVersion() : QString();
        m_checker->check(currentVersion(), KemaiUpdateChecker::channelOf(channel()), ignored)
            .then(this,
                  [this, generation](const std::optional<KemaiRelease> &release)
                  {
                      if (generation != m_generation)
                      {
                          return;
                      }
                      if (!m_preferences.isNull())
                      {
                          m_preferences->setLastUpdateCheck(QDateTime::currentDateTimeUtc());
                      }
                      spdlog::info("Update check: {}", release ? release->version : QString("up to date"));
                      m_latest = release.value_or(KemaiRelease{});
                      setState(release ? Available : m_source == Startup ? Idle :
                                                                           UpToDate);
                  })
            .onFailed(this,
                      [this, generation](const UpdateCheckError &e)
                      {
                          if (generation != m_generation)
                          {
                              return;
                          }
                          // Logged by the checker; silent at start
                          m_failure = e.kind() == UpdateCheckError::Kind::NoNetwork ? NoNetwork : e.kind() == UpdateCheckError::Kind::ServerError ? ServerError :
                                                                                                                                                    InvalidResponse;
                          setState(m_source == Startup ? Idle : Failed);
                      });
    }

    void UpdateNotifier::setState(Status status)
    {
        m_status = status;
        emit stateChanged();
    }
} // namespace kemai
