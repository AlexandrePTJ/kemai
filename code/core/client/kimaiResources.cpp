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
        QJsonObject jo;

        if (id > 0)
        {
            jo["id"] = id;
        }

        jo["name"]       = name;
        jo["number"]     = number;
        jo["comment"]    = comment;
        jo["company"]    = company;
        jo["address"]    = address;
        jo["country"]    = countryKey;
        jo["currency"]   = currencyKey;
        jo["phone"]      = phone;
        jo["fax"]        = fax;
        jo["mobile"]     = mobile;
        jo["email"]      = email;
        jo["homepage"]   = homepage;
        jo["timezone"]   = timezone;
        jo["budget"]     = budget;
        jo["timeBudget"] = timeBudget;
        jo["visible"]    = visible;
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
        QJsonObject jo;

        if (id > 0)
        {
            jo["id"] = id;
        }

        jo["name"]        = name;
        jo["visible"]     = visible;
        jo["comment"]     = comment;
        jo["orderNumber"] = orderNumber;
        jo["orderDate"]   = orderDate;
        jo["start"]       = start;
        jo["end"]         = end;
        jo["color"]       = color;
        jo["customer"]    = customer.id;
        jo["budget"]      = budget;
        jo["timeBudget"]  = timeBudget;
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
        QJsonObject jo;

        jo["name"]    = name;
        jo["visible"] = visible;
        jo["comment"] = comment;
        if (project)
        {
            jo["project"] = project->id;
        }
        jo["budget"]     = budget;
        jo["timeBudget"] = timeBudget;
        return jo;
    }

} // namespace kemai
