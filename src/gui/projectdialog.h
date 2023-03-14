#pragma once

#include <QDialog>

#include "client/kimaiclient.h"

namespace Ui {
class ProjectDialog;
}

namespace kemai {

class ProjectDialog : public QDialog
{
    Q_OBJECT

public:
    ProjectDialog(QWidget* parent = nullptr);
    ~ProjectDialog() override;

    Project project() const;

private:
    void enableSave(bool enable);
    void validateForm();

private:
    Ui::ProjectDialog* mUi;
};

} // namespace kemai
