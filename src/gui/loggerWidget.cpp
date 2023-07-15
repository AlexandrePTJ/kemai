#include "loggerWidget.h"
#include "ui_loggerWidget.h"

#include <QDesktopServices>

#include <misc/helpers.h>

using namespace kemai;

LoggerWidget::LoggerWidget(QWidget* parent) : QWidget(parent), mUi(std::make_unique<Ui::LoggerWidget>())
{
    mUi->setupUi(this);
    mUi->leLogFilePath->setText(helpers::getLogFilePath());

    connect(mUi->tbOpenExplorerToLog, &QToolButton::clicked, []() { QDesktopServices::openUrl(QUrl(QString("file:///%1").arg(helpers::getLogDirPath()))); });
}

LoggerWidget::~LoggerWidget() = default;

void LoggerWidget::setModel(const std::shared_ptr<LoggerTreeModel>& model)
{
    mLoggerModel = model;
    mUi->tvLogView->setModel(mLoggerModel.get());
}
