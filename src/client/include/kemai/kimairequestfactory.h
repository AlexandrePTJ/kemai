#pragma once

#include "kimairequest.h"

namespace kemai::client {

class KimaiRequestFactory
{
public:
    static KimaiRequest version();
    static KimaiRequest customers();
    static KimaiRequest customerAdd(const Customer& customer);
    static KimaiRequest projects(int customerId);
//    static KimaiRequest projectAdd(const QString& name, int customerId);
    static KimaiRequest activities(int projectId);
//    static KimaiRequest activityAdd(const QString& name);
    static KimaiRequest activeTimeSheets();
    static KimaiRequest startTimeSheet(int projectId, int activityId, const QDateTime& beginAt);
    static KimaiRequest stopTimeSheet(int timeSheetId);
};

} // namespace kemai::client
