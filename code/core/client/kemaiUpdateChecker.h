// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// STL headers
#include <chrono>
#include <memory>
#include <optional>

// Qt headers
#include <QException>
#include <QFuture>
#include <QJsonDocument>
#include <QObject>
#include <QString>
#include <QUrl>

class QNetworkAccessManager;

namespace kemai
{
    // A published Kemai release
    struct KemaiRelease
    {
        QString version; // calendar version, e.g. "2026.12.1"
        QUrl    url;     // its GitHub page
        bool    prerelease = false;
    };

    // Why a check failed (no user-facing text: the UI words it)
    class UpdateCheckError : public QException
    {
    public:
        enum class Kind
        {
            NoNetwork,      // no answer at all (offline, DNS, timeout)
            ServerError,    // an HTTP error (rate limit, GitHub down)
            InvalidResponse // not a releases list
        };

        UpdateCheckError(Kind kind, QString detail);

        Kind    kind() const;
        QString detail() const;

        void              raise() const override;
        UpdateCheckError *clone() const override;
        const char       *what() const noexcept override;

    private:
        Kind       m_kind;
        QString    m_detail;
        QByteArray m_what;
    };

    /*!
     * Asks GitHub for the releases of Kemai and tells the newest one of the running channel above the running
     * version: an alpha (or development) build considers pre-releases and releases, a stable build releases only.
     * Drafts and tags that are not calendar versions (0.11.x and older) are ignored.
     */
    class KemaiUpdateChecker : public QObject
    {
        Q_OBJECT

    public:
        static constexpr std::chrono::milliseconds DefaultTransferTimeout{15000};

        enum class Channel
        {
            Alpha,
            Stable,
            Dev // no comparable version: every release of the alpha channel is newer
        };

        explicit KemaiUpdateChecker(QObject *parent = nullptr);
        ~KemaiUpdateChecker() override;

        static Channel channelOf(const QString &name);
        // GitHub's releases list of KEMAI_REPOSITORY; tests point it at a local server
        static QUrl defaultReleasesUrl();
        void        setReleasesUrl(const QUrl &url);

        // Newest release above `currentVersion` and above `ignoredVersion` (when not empty), nullopt if none.
        // Fails with UpdateCheckError.
        QFuture<std::optional<KemaiRelease>> check(const QString &currentVersion, Channel channel, const QString &ignoredVersion = {});

        static bool isCalendarVersion(const QString &version);
        // Same decision on a releases list already fetched; throws UpdateCheckError (InvalidResponse) if it is not one
        static std::optional<KemaiRelease> newestRelease(const QJsonDocument &releases, const QString &currentVersion, Channel channel, const QString &ignoredVersion = {});

    private:
        std::unique_ptr<QNetworkAccessManager> m_networkAccessManager;
        QUrl                                   m_releasesUrl;
    };
} // namespace kemai
