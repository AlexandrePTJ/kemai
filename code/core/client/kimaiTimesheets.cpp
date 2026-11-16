// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "kimaiTimesheets.h"

// Project headers
#include <misc/jsonHelpers.h>

namespace kemai
{
    /*
     * Static helpers
     */
    static KimaiTimeSheetConfig::TrackingMode trackingModeFromString(const QString &trackingMode)
    {
        if (trackingMode == "duration_fixed_begin")
        {
            return KimaiTimeSheetConfig::TrackingMode::DurationFixedBegin;
        }
        if (trackingMode == "duration_only")
        {
            return KimaiTimeSheetConfig::TrackingMode::DurationOnly;
        }
        if (trackingMode == "punch")
        {
            return KimaiTimeSheetConfig::TrackingMode::Punch;
        }
        return KimaiTimeSheetConfig::TrackingMode::Default;
    }

    static KimaiTask::Status taskStatusFromString(const QString &taskStatus)
    {
        if (taskStatus == "pending")
        {
            return KimaiTask::Status::Pending;
        }
        if (taskStatus == "progress")
        {
            return KimaiTask::Status::Progress;
        }
        if (taskStatus == "closed")
        {
            return KimaiTask::Status::Closed;
        }
        return KimaiTask::Status::Undefined;
    }

    /*
     * KimaiTimeSheetConfig
     */
    KimaiTimeSheetConfig KimaiTimeSheetConfig::fromJson(const QJsonValue &json)
    {
        JsonHelpers::checkTypeOrThrow("KimaiTimeSheetConfig", json, {QJsonValue::Object});

        auto jsonObject = json.toObject();
        JsonHelpers::checkKeysOrThrow("KimaiTimeSheetConfig", jsonObject, {"trackingMode"});

        KimaiTimeSheetConfig config;
        config.trackingMode = trackingModeFromString(jsonObject.value("trackingMode").toString());
        return config;
    }

    /*
     * KimaiMetaFieldRule
     */
    KimaiMetaFieldRule KimaiMetaFieldRule::fromJson(const QJsonValue &json)
    {
        JsonHelpers::checkTypeOrThrow("KimaiMetaFieldRule", json, {QJsonValue::Object});

        auto jsonObject = json.toObject();
        JsonHelpers::checkKeysOrThrow("KimaiMetaFieldRule", jsonObject, {"name", "type"});

        // Restrictions are ids, or null when unrestricted
        const auto restriction = [&jsonObject](const char *key)
        { return jsonObject.value(key).toInt(); };

        KimaiMetaFieldRule rule;
        rule.name       = jsonObject.value("name").toString();
        rule.label      = jsonObject.value("label").toString();
        rule.help       = jsonObject.value("help").toString();
        rule.type       = jsonObject.value("type").toString();
        rule.rawValue   = jsonObject.value("value").toVariant().toString();
        rule.required   = jsonObject.value("required").toBool();
        rule.customerId = restriction("customer");
        rule.projectId  = restriction("project");
        rule.activityId = restriction("activity");
        return rule;
    }

    /*
     * KimaiTimeSheet
     */
    KimaiTimeSheet KimaiTimeSheet::fromJson(const QJsonValue &json)
    {
        JsonHelpers::checkTypeOrThrow("KimaiTimeSheet", json, {QJsonValue::Object, QJsonValue::Double});

        if (json.isDouble())
        {
            KimaiTimeSheet timeSheet;
            timeSheet.id = json.toInt();
            return timeSheet;
        }

        auto jsonObject = json.toObject();
        JsonHelpers::checkKeysOrThrow("KimaiTimeSheet", jsonObject, {"id", "begin"});

        KimaiTimeSheet timeSheet;
        timeSheet.id          = jsonObject.value("id").toInt();
        timeSheet.project     = KimaiProject::fromJson(jsonObject.value("project"));
        timeSheet.activity    = KimaiActivity::fromJson(jsonObject.value("activity"));
        timeSheet.description = jsonObject.value("description").toString();
        timeSheet.beginAt     = QDateTime::fromString(jsonObject.value("begin").toString(), Qt::ISODate);
        timeSheet.endAt       = QDateTime::fromString(jsonObject.value("end").toString(), Qt::ISODate);
        timeSheet.user        = jsonObject.value("user").toInt();

        for (const auto &jsTag : jsonObject.value("tags").toArray())
        {
            timeSheet.tags.push_back(jsTag.toString());
        }

        // [{name, value}]; an unfilled field has a null value
        for (const auto &jvMeta : jsonObject.value("metaFields").toArray())
        {
            const auto meta = jvMeta.toObject();
            const auto name = meta.value("name").toString();
            if (!name.isEmpty())
            {
                timeSheet.metaFields.insert(name, meta.value("value").toVariant().toString());
            }
        }

        return timeSheet;
    }

    QJsonObject KimaiTimeSheet::toJson(KimaiTimeSheetConfig::TrackingMode trackingMode) const
    {
        QJsonObject jo;

        if (trackingMode != KimaiTimeSheetConfig::TrackingMode::Punch)
        {
            jo["begin"] = beginAt.toString(Qt::ISODate);
            if (endAt.isValid())
            {
                jo["end"] = endAt.toString(Qt::ISODate);
            }
        }

        jo["project"]     = project.id;
        jo["activity"]    = activity.id;
        jo["description"] = description;
        jo["tags"]        = tags.join(',');
        return jo;
    }

    /*
     * KimaiTask
     */
    KimaiTask KimaiTask::fromJson(const QJsonValue &json)
    {
        JsonHelpers::checkTypeOrThrow("KimaiTask", json, {QJsonValue::Object, QJsonValue::Double});

        if (json.isDouble())
        {
            KimaiTask task;
            task.id = json.toInt();
            return task;
        }

        auto jsonObject = json.toObject();
        JsonHelpers::checkKeysOrThrow("KimaiTask", jsonObject, {"id", "title"});

        KimaiTask task;
        task.id          = jsonObject.value("id").toInt();
        task.title       = jsonObject.value("title").toString();
        task.status      = taskStatusFromString(jsonObject.value("status").toString());
        task.todo        = jsonObject.value("todo").toString();
        task.description = jsonObject.value("description").toString();
        task.project     = KimaiProject::fromJson(jsonObject.value("project"));
        task.activity    = KimaiActivity::fromJson(jsonObject.value("activity"));
        task.user        = KimaiUser::fromJson(jsonObject.value("user"));
        task.endAt       = QDateTime::fromString(jsonObject.value("end").toString(), Qt::ISODate);
        task.estimation  = jsonObject.value("estimation").toInt();

        if (jsonObject.contains("activeTimesheets"))
        {
            for (const auto &jvTimeSheet : jsonObject.value("activeTimesheets").toArray())
            {
                task.activeTimeSheets.push_back(KimaiTimeSheet::fromJson(jvTimeSheet));
            }
        }

        return task;
    }

} // namespace kemai
