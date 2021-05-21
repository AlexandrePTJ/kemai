#pragma once

#include <QDialog>

#include "client/kimaiapi.h"

namespace Ui {
class CustomerDialog;
}

namespace kemai::app {

class CustomerDialog : public QDialog
{
    Q_OBJECT

public:
    CustomerDialog(QWidget* parent = nullptr);
    ~CustomerDialog() override;

    client::Customer customer() const;

private:
    void enableSave(bool enable);
    void validateForm();

private:
    Ui::CustomerDialog* mUi;
};

} // namespace kemai::app
