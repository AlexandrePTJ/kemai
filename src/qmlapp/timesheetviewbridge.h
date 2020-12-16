#pragma once

#include <QObject>

#include "activityfiltermodel.h"
#include "projectfiltermodel.h"

namespace kemai::qmlapp {

class TimesheetViewBridge : public QObject
{
    Q_OBJECT

public:
    TimesheetViewBridge();
    virtual ~TimesheetViewBridge() = default;

    void setProjectModel(QAbstractListModel* model);
    ProjectFilterModel* projectFilterModel();

    void setActivityModel(QAbstractListModel* model);
    ActivityFilterModel* activityFilterModel();

private:
    ProjectFilterModel mProjectFilterModel;
    ActivityFilterModel mActivityFilterModel;
};

} // namespace kemai::qmlapp
