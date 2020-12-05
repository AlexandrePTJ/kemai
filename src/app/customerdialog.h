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
    ~CustomerDialog();

    void setCustomer(const client::Customer& customer);
    client::Customer customer() const;

private:
    void enableSave(bool enable);

private slots:
    void validateForm();

private:
    Ui::CustomerDialog* mUi;
};

} // namespace kemai::app
