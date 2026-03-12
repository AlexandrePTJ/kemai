// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// Qt headers
#include <QList>
#include <QStringList>

// Internal headers
#include <client/kimaiResources.h>
#include <client/kimaiSystem.h>

namespace kemai
{

    struct KimaiTimeSheetConfig
    {
        enum class TrackingMode
        {
            Default,
            Punch,
            DurationFixedBegin,
            DurationOnly
        };

        TrackingMode trackingMode = TrackingMode::Default;

        static KimaiTimeSheetConfig fromJson(const QJsonValue &json);
    };

    struct KimaiTimeSheet
    {
        int           id   = 0;
        int           user = 0;
        KimaiActivity activity;
        KimaiProject  project;
        QString       description;
        QDateTime     beginAt;
        QDateTime     endAt;
        QStringList   tags;

        static KimaiTimeSheet fromJson(const QJsonValue &json);
        QJsonObject           toJson(KimaiTimeSheetConfig::TrackingMode trackingMode) const;
    };
    using KimaiTimeSheets = QList<KimaiTimeSheet>;

    struct KimaiTask
    {
        enum class Status
        {
            Undefined,
            Pending,
            Progress,
            Closed
        };

        int             id = 0;
        QString         title;
        Status          status = Status::Undefined;
        QString         todo;
        QString         description;
        KimaiProject    project;
        KimaiActivity   activity;
        KimaiUser       user;
        QDateTime       endAt;
        int             estimation = 0;
        KimaiTimeSheets activeTimeSheets;

        static KimaiTask fromJson(const QJsonValue &json);
    };
    using KimaiTasks = QList<KimaiTask>;

} // namespace kemai
