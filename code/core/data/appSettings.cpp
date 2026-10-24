#include "appSettings.h"

// Qt headers
#include <QJsonArray>

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
        if (!preferences.isEmpty())
        {
            json["preferences"] = preferences;
        }
        if (!trustedCertificates.isEmpty())
        {
            QJsonArray certificates;
            for (const auto &certificate : trustedCertificates)
            {
                certificates.append(QJsonObject{
                    {"host",      certificate.host                                 },
                    {"sha256",    certificate.sha256                               },
                    {"pem",       QString::fromLatin1(certificate.pem)             },
                    {"expiresAt", certificate.expiresAt.toString(Qt::ISODate)      }
                });
            }
            json["trustedCertificates"] = certificates;
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
        settings.preferences = json["preferences"].toObject();
        for (const auto &value : json["trustedCertificates"].toArray())
        {
            const auto certificate = value.toObject();
            settings.trustedCertificates.append(TrustedCertificate{
                .host      = certificate["host"].toString(),
                .sha256    = certificate["sha256"].toString(),
                .pem       = certificate["pem"].toString().toLatin1(),
                .expiresAt = QDateTime::fromString(certificate["expiresAt"].toString(), Qt::ISODate),
            });
        }
        return settings;
    }

} // namespace kemai
