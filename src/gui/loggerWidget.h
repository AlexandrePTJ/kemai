#pragma once

#include <memory>

#include <QWidget>

#include <models/loggerTreeModel.h>

namespace Ui {
class LoggerWidget;
}

namespace kemai {

class LoggerWidget : public QWidget
{
public:
    LoggerWidget(QWidget* parent = nullptr);
    ~LoggerWidget() override;

    void setModel(const std::shared_ptr<LoggerTreeModel>& model);

private:
    std::unique_ptr<Ui::LoggerWidget> mUi;
    std::shared_ptr<LoggerTreeModel> mLoggerModel;
};

} // namespace kemai
