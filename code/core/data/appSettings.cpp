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
        return settings;
    }

} // namespace kemai
