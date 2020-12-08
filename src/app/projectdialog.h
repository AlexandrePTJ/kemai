#pragma once

#include <QDialog>

#include "client/kimaiclient.h"

namespace Ui {
class ProjectDialog;
}

namespace kemai::app {

class ProjectDialog : public QDialog
{
    Q_OBJECT

public:
    ProjectDialog(QWidget* parent = nullptr);
    ~ProjectDialog();

    void setProject(const client::Project& project);
    client::Project project() const;

private:
    void enableSave(bool enable);

private slots:
    void validateForm();

private:
    Ui::ProjectDialog* mUi;
};

} // namespace kemai::app
