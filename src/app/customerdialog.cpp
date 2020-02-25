#include "customerdialog.h"
#include "ui_customerdialog.h"

#include <QPushButton>

using namespace kemai::app;
using namespace kemai::client;

CustomerDialog::CustomerDialog(QWidget* parent) : QDialog(parent), mUi(new Ui::CustomerDialog)
{
    mUi->setupUi(this);
    enableSave(false);

    connect(mUi->leName, &QLineEdit::textChanged, this, &CustomerDialog::validateForm);
}

CustomerDialog::~CustomerDialog()
{
    delete mUi;
}

void CustomerDialog::setCustomer(const Customer& customer)
{
    mUi->leName->setText(customer.name);
    validateForm();
}

Customer CustomerDialog::customer() const
{
    Customer customer;
    customer.name = mUi->leName->text();
    return customer;
}

void CustomerDialog::enableSave(bool enable)
{
    auto btn = mUi->buttonBox->button(QDialogButtonBox::Save);
    if (btn)
        btn->setEnabled(enable);
}

void CustomerDialog::validateForm()
{
    bool nameOk = not mUi->leName->text().isEmpty();
    enableSave(nameOk);
}
