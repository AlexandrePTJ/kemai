// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "kemaiUpdateChecker.h"

// Qt headers
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPromise>
#include <QRegularExpression>
#include <QVersionNumber>

// 3rd party headers
#include <spdlog/spdlog.h>

// Project headers
#include "kemaiConfig.h"
#include "misc/customFmt.h"

namespace kemai
{
    UpdateCheckError::UpdateCheckError(Kind kind, QString detail):
    m_kind(kind),
    m_detail(std::move(detail)),
    m_what(m_detail.toUtf8())
    {
    }

    UpdateCheckError::Kind UpdateCheckError::kind() const
    {
        return m_kind;
    }

    QString UpdateCheckError::detail() const
    {
        return m_detail;
    }

    void UpdateCheckError::raise() const
    {
        throw *this;
    }

    UpdateCheckError *UpdateCheckError::clone() const
    {
        return new UpdateCheckError(*this);
    }

    const char *UpdateCheckError::what() const noexcept
    {
        return m_what.constData();
    }

    KemaiUpdateChecker::KemaiUpdateChecker(QObject *parent):
    QObject(parent),
    m_networkAccessManager(std::make_unique<QNetworkAccessManager>()),
    m_releasesUrl(defaultReleasesUrl())
    {
        // A connection whose network went away never errors on its own
        m_networkAccessManager->setTransferTimeout(DefaultTransferTimeout);
    }

    KemaiUpdateChecker::~KemaiUpdateChecker() = default;

    KemaiUpdateChecker::Channel KemaiUpdateChecker::channelOf(const QString &name)
    {
        if (name == "stable")
        {
            return Channel::Stable;
        }
        return name == "alpha" ? Channel::Alpha : Channel::Dev;
    }

    QUrl KemaiUpdateChecker::defaultReleasesUrl()
    {
        return QUrl(QString("https://api.github.com/repos/%1/releases?per_page=20").arg(KEMAI_REPOSITORY));
    }

    void KemaiUpdateChecker::setReleasesUrl(const QUrl &url)
    {
        m_releasesUrl = url;
    }

    QFuture<std::optional<KemaiRelease>> KemaiUpdateChecker::check(const QString &currentVersion, Channel channel, const QString &ignoredVersion)
    {
        auto promise = std::make_shared<QPromise<std::optional<KemaiRelease>>>();
        promise->start();

        QNetworkRequest request(m_releasesUrl);
        request.setRawHeader("Accept", "application/vnd.github+json");
        request.setHeader(QNetworkRequest::UserAgentHeader, QString("Kemai/%1").arg(KEMAI_VERSION));

        auto *reply = m_networkAccessManager->get(request);
        connect(reply,
                &QNetworkReply::finished,
                this,
                [reply, promise, currentVersion, channel, ignoredVersion]
                {
                    reply->deleteLater();
                    const auto httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                    try
                    {
                        if (httpStatus == 0)
                        {
                            throw UpdateCheckError(UpdateCheckError::Kind::NoNetwork, reply->errorString());
                        }
                        if (httpStatus >= 400 || reply->error() != QNetworkReply::NoError)
                        {
                            throw UpdateCheckError(UpdateCheckError::Kind::ServerError, QString("HTTP %1").arg(httpStatus));
                        }
                        promise->addResult(newestRelease(QJsonDocument::fromJson(reply->readAll()), currentVersion, channel, ignoredVersion));
                    }
                    catch (const UpdateCheckError &e)
                    {
                        spdlog::warn("Update check failed: {}", e.detail());
                        promise->setException(e);
                    }
                    promise->finish();
                });

        return promise->future();
    }

    bool KemaiUpdateChecker::isCalendarVersion(const QString &version)
    {
        // Same rule as the release workflow (.github/scripts/release-channel.sh)
        static const QRegularExpression calendar(R"(^\d{2}\.(1[0-2]|[1-9])\.\d+$)");
        return calendar.match(version).hasMatch();
    }

    std::optional<KemaiRelease> KemaiUpdateChecker::newestRelease(const QJsonDocument &releases, const QString &currentVersion, Channel channel, const QString &ignoredVersion)
    {
        if (!releases.isArray())
        {
            throw UpdateCheckError(UpdateCheckError::Kind::InvalidResponse, "not a releases list");
        }

        // A development build has no comparable version: any release is newer
        const auto current = channel == Channel::Dev ? QVersionNumber() : QVersionNumber::fromString(currentVersion);

        std::optional<KemaiRelease> newest;
        QVersionNumber              newestNumber;
        for (const auto &value : releases.array())
        {
            const auto release = value.toObject();
            if (!release.contains("tag_name"))
            {
                throw UpdateCheckError(UpdateCheckError::Kind::InvalidResponse, "release without tag");
            }

            const auto tag        = release.value("tag_name").toString();
            const auto prerelease = release.value("prerelease").toBool();
            if (release.value("draft").toBool() || !isCalendarVersion(tag) || (prerelease && channel == Channel::Stable))
            {
                continue;
            }

            const auto number = QVersionNumber::fromString(tag);
            if (number > current && number > newestNumber)
            {
                newestNumber = number;
                newest       = KemaiRelease{.version = tag, .url = QUrl(release.value("html_url").toString()), .prerelease = prerelease};
            }
        }

        // An ignored version also silences the older ones: the user already chose not to follow it
        if (newest && !ignoredVersion.isEmpty() && newestNumber <= QVersionNumber::fromString(ignoredVersion))
        {
            return std::nullopt;
        }
        return newest;
    }
} // namespace kemai
