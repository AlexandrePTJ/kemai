// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// Qt headers
#include <QJsonObject>
#include <QString>
#include <QUuid>

namespace kemai
{
    using ProfileId = QUuid;

    struct Profile
    {
        ProfileId id;
        QString   name;
        QString   host;
        QString   token;

        bool           isValid() const;
        QJsonObject    toJson() const;
        static Profile fromJson(const QJsonObject &json);
    };

} // namespace kemai
