#include "timesheetviewbridge.h"

using namespace kemai::qmlapp;

TimesheetViewBridge::TimesheetViewBridge() {}

void TimesheetViewBridge::setProjectModel(QAbstractListModel* model)
{
    mProjectFilterModel.setSourceModel(model);
}

ProjectFilterModel* TimesheetViewBridge::projectFilterModel()
{
    return &mProjectFilterModel;
}

void TimesheetViewBridge::setActivityModel(QAbstractListModel* model)
{
    mActivityFilterModel.setSourceModel(model);
}

ActivityFilterModel* TimesheetViewBridge::activityFilterModel()
{
    return &mActivityFilterModel;
}
