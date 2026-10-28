// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// Qt headers
#include <QtQmlIntegration/qqmlintegration.h>

// Project headers
#include "referenceData.h"
#include <context/sessionController.h>

namespace kemai
{
    class SessionContext : public SessionController
    {
        Q_OBJECT
        QML_ELEMENT
        QML_UNCREATABLE("SessionContext is created by the application")

    public:
        SessionContext(std::shared_ptr<IKimaiClient> client, const KimaiUser &user, QObject *parent = nullptr):
        SessionController(std::move(client), user, parent)
        {
            setKnownCodes(ReferenceData::knownCodes());
        }
    };

    // QML registrations for core types used as QML property types
    struct SessionControllerForeign
    {
        Q_GADGET
        QML_FOREIGN(SessionController)
        QML_NAMED_ELEMENT(SessionController)
        QML_UNCREATABLE("SessionController is exposed through SessionContext")
    };

    struct ActivityListModelForeign
    {
        Q_GADGET
        QML_FOREIGN(ActivityListModel)
        QML_NAMED_ELEMENT(ActivityListModel)
        QML_UNCREATABLE("ActivityListModel is created by the application")
    };

    struct TimesheetModelForeign
    {
        Q_GADGET
        QML_FOREIGN(TimesheetModel)
        QML_NAMED_ELEMENT(TimesheetModel)
        QML_UNCREATABLE("TimesheetModel is created by the application")
    };

    struct TaskModelForeign
    {
        Q_GADGET
        QML_FOREIGN(TaskModel)
        QML_NAMED_ELEMENT(TaskModel)
        QML_UNCREATABLE("TaskModel is created by the application")
    };
} // namespace kemai
