#include "appSettings.h"

namespace kemai
{

    QJsonObject AppSettings::toJson() const
    {
        QJsonObject json;
        if (autoLogin.has_value())
        {
            json["autoLogin"] = QJsonObject{
                {"host", autoLogin->host}
            };
        }
        if (window.has_value())
        {
            json["window"] = QJsonObject{
                {"x",         window->normal.x()     },
                {"y",         window->normal.y()     },
                {"width",     window->normal.width() },
                {"height",    window->normal.height()},
                {"maximized", window->maximized      }
            };
        }
        return json;
    }

    AppSettings AppSettings::fromJson(const QJsonObject &json)
    {
        AppSettings settings;
        if (json.contains("autoLogin"))
        {
            // Note: we intentionally don't store the token in the settings file, so we don't load it here.
            settings.autoLogin = Credentials{
                .host = json["autoLogin"].toObject()["host"].toString(),
            };
        }
        if (json.contains("window"))
        {
            const auto window = json["window"].toObject();
            const QRect normal(window["x"].toInt(), window["y"].toInt(), window["width"].toInt(), window["height"].toInt());
            if (normal.isValid())
            {
                settings.window = WindowGeometry{.normal = normal, .maximized = window["maximized"].toBool()};
            }
        }
        return settings;
    }

} // namespace kemai
