#include "projectdialog.h"
#include "ui_projectdialog.h"

#include "datareader.h"

#include <QPushButton>

using namespace kemai::app;
using namespace kemai::client;

ProjectDialog::ProjectDialog(QWidget* parent) : QDialog(parent), mUi(new Ui::ProjectDialog)
{
    mUi->setupUi(this);
    enableSave(false);
}

ProjectDialog::~ProjectDialog()
{
    delete mUi;
}

void ProjectDialog::setCustomers(const Customers& customers)
{
    mUi->cbCustomer->clear();

    for(const auto& customer : customers)
    {
        mUi->cbCustomer->addItem(customer.name, customer.id);
    }
}

void ProjectDialog::setProject(const Project& project)
{
    validateForm();
}

Project ProjectDialog::project() const
{
    Project project;
    return project;
}

void ProjectDialog::enableSave(bool enable)
{
//    auto btn = mUi->buttonBox->button(QDialogButtonBox::Save);
//    if (btn)
//        btn->setEnabled(enable);
}

void ProjectDialog::validateForm()
{
//    bool nameOk = not mUi->leName->text().isEmpty();
//    enableSave(nameOk);
}
