// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "kimaiResources.h"

// Project headers
#include <misc/jsonHelpers.h>

namespace kemai
{
    QString getColorSafe(const QJsonObject &jsonObject)
    {
        if (jsonObject.contains("color-safe"))
        {
            return jsonObject.value("color-safe").toString();
        }
        else
        {
            return jsonObject.value("color").toString();
        }
    }

    KimaiCustomer KimaiCustomer::fromJson(const QJsonValue &json)
    {
        JsonHelpers::checkTypeOrThrow("KimaiCustomer", json, {QJsonValue::Object, QJsonValue::Double});

        if (json.isDouble())
        {
            KimaiCustomer customer;
            customer.id = json.toInt();
            return customer;
        }

        auto jsonObject = json.toObject();
        JsonHelpers::checkKeysOrThrow("KimaiCustomer", jsonObject, {"id"});

        KimaiCustomer customer;
        customer.id          = jsonObject.value("id").toInt();
        customer.name        = jsonObject.value("name").toString();
        customer.number      = jsonObject.value("number").toString();
        customer.comment     = jsonObject.value("comment").toString();
        customer.company     = jsonObject.value("company").toString();
        customer.address     = jsonObject.value("address").toString();
        customer.countryKey  = jsonObject.value("country").toString();
        customer.currencyKey = jsonObject.value("currency").toString();
        customer.phone       = jsonObject.value("phone").toString();
        customer.fax         = jsonObject.value("fax").toString();
        customer.mobile      = jsonObject.value("mobile").toString();
        customer.email       = jsonObject.value("email").toString();
        customer.homepage    = jsonObject.value("homepage").toString();
        customer.timezone    = jsonObject.value("timezone").toString();
        customer.color       = getColorSafe(jsonObject);
        customer.budget      = jsonObject.value("budget").toDouble();
        customer.timeBudget  = jsonObject.value("timeBudget").toInt();
        customer.visible     = jsonObject.value("visible").toBool();
        return customer;
    }

    QJsonObject KimaiCustomer::toJson() const
    {
        // Kimai rejects unknown fields ("extra fields") and validates every one sent: only what creation needs
        QJsonObject jo{
            {"name",     name       },
            {"country",  countryKey },
            {"currency", currencyKey},
            {"timezone", timezone   },
            {"visible",  visible    }
        };
        if (!comment.isEmpty())
        {
            jo["comment"] = comment;
        }
        return jo;
    }

    KimaiProject KimaiProject::fromJson(const QJsonValue &json)
    {
        JsonHelpers::checkTypeOrThrow("KimaiProject", json, {QJsonValue::Object, QJsonValue::Double});

        if (json.isDouble())
        {
            KimaiProject project;
            project.id = json.toInt();
            return project;
        }

        auto jsonObject = json.toObject();
        JsonHelpers::checkKeysOrThrow("KimaiProject", jsonObject, {"id", "name"});

        KimaiProject project;
        project.id          = jsonObject.value("id").toInt();
        project.name        = jsonObject.value("name").toString();
        project.comment     = jsonObject.value("comment").toString();
        project.orderNumber = jsonObject.value("orderNumber").toString();
        project.orderDate   = jsonObject.value("orderDate").toString();
        project.start       = jsonObject.value("start").toString();
        project.end         = jsonObject.value("end").toString();
        project.color       = getColorSafe(jsonObject);
        project.budget      = jsonObject.value("budget").toDouble();
        project.timeBudget  = jsonObject.value("timeBudget").toInt();
        project.visible     = jsonObject.value("visible").toBool();

        if (jsonObject.contains("customer"))
        {
            project.customer = KimaiCustomer::fromJson(jsonObject.value("customer"));
        }

        return project;
    }

    QJsonObject KimaiProject::toJson() const
    {
        QJsonObject jo{
            {"name",     name       },
            {"customer", customer.id},
            {"visible",  visible    }
        };
        if (!comment.isEmpty())
        {
            jo["comment"] = comment;
        }
        return jo;
    }

    KimaiActivity KimaiActivity::fromJson(const QJsonValue &json)
    {
        JsonHelpers::checkTypeOrThrow("KimaiActivity", json, {QJsonValue::Object, QJsonValue::Double});

        if (json.isDouble())
        {
            KimaiActivity activity;
            activity.id = json.toInt();
            return activity;
        }

        auto jsonObject = json.toObject();
        JsonHelpers::checkKeysOrThrow("KimaiActivity", jsonObject, {"id", "name"});

        KimaiActivity activity;
        activity.id         = jsonObject.value("id").toInt();
        activity.name       = jsonObject.value("name").toString();
        activity.comment    = jsonObject.value("comment").toString();
        activity.color      = jsonObject.value("color-safe").toString();
        activity.budget     = jsonObject.value("budget").toDouble();
        activity.timeBudget = jsonObject.value("timeBudget").toInt();
        activity.visible    = jsonObject.value("visible").toBool();

        if (jsonObject.contains("project") && !jsonObject.value("project").isNull())
        {
            activity.project = KimaiProject::fromJson(jsonObject.value("project"));
        }

        return activity;
    }

    QJsonObject KimaiActivity::toJson() const
    {
        QJsonObject jo{
            {"name",    name   },
            {"visible", visible}
        };
        // No project: a global activity
        if (project)
        {
            jo["project"] = project->id;
        }
        if (!comment.isEmpty())
        {
            jo["comment"] = comment;
        }
        return jo;
    }

} // namespace kemai
