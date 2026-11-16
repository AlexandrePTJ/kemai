// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// Qt headers
#include <QList>
#include <QMap>
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

    // A custom field of time entries (MetaFieldsBundle plugin), as `GET metafields?entity=timesheet` lists it for the
    // user. A restriction of 0 means unrestricted.
    struct KimaiMetaFieldRule
    {
        QString name;
        QString label;
        QString help;
        QString type;     // Kimai's type name: text, integer, choice, date...
        QString rawValue; // default value; for a choice, its options ("value|label" entries, comma-separated)
        bool    required   = false;
        int     customerId = 0;
        int     projectId  = 0;
        int     activityId = 0;

        static KimaiMetaFieldRule fromJson(const QJsonValue &json);
    };
    using KimaiMetaFieldRules = QList<KimaiMetaFieldRule>;

    // A color of the instance's palette (GET config/colors), offered by color custom fields
    struct KimaiColorChoice
    {
        QString name;
        QString color; // #rrggbb, as Kimai stores it (case kept)

        bool operator==(const KimaiColorChoice &) const = default;
    };
    using KimaiColorChoices = QList<KimaiColorChoice>;

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
        // Custom field values (name -> value, empty when not filled), as Kimai replies them. Never sent with the
        // entry: Kimai's timesheet form refuses them; they are written one by one through `PATCH timesheets/{id}/meta`.
        QMap<QString, QString> metaFields;

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
