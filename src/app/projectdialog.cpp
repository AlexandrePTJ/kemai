#include "projectdialog.h"
#include "ui_projectdialog.h"

#include "helpers.h"

#include "kemai/kimairequestfactory.h"

#include <QPushButton>
#include <QTimer>

using namespace kemai::app;
using namespace kemai::client;

ProjectDialog::ProjectDialog(QWidget* parent) : QDialog(parent), mUi(new Ui::ProjectDialog)
{
    mUi->setupUi(this);
    enableSave(false);

    mClient = helpers::createClient();
    if (mClient)
    {
        connect(mClient.data(), &KimaiClient::replyReceived, this, &ProjectDialog::onClientReply);
        QTimer::singleShot(200, [=]() { mClient->sendRequest(KimaiRequestFactory::customers()); });
    }

    connect(mUi->leName, &QLineEdit::textChanged, this, &ProjectDialog::validateForm);
    connect(mUi->cbCustomer, &QComboBox::currentTextChanged, this, &ProjectDialog::validateForm);
    connect(mUi->leTimeBudget, &QLineEdit::textChanged, this, &ProjectDialog::validateForm);
}

ProjectDialog::~ProjectDialog()
{
    delete mUi;
}

void ProjectDialog::setProject(const Project& project)
{
    validateForm();
}

Project ProjectDialog::project() const
{
    Project project;
    project.id          = 0;
    project.visible     = true;
    project.name        = mUi->leName->text();
    project.customer.id = mUi->cbCustomer->currentData(Qt::UserRole).toInt();
    project.budget      = mUi->sbBudget->value();
    project.timeBudget  = mUi->leTimeBudget->seconds();
    return project;
}

void ProjectDialog::enableSave(bool enable)
{
    auto btn = mUi->buttonBox->button(QDialogButtonBox::Save);
    if (btn)
        btn->setEnabled(enable);
}

void ProjectDialog::onClientReply(const KimaiReply& reply)
{
    switch (reply.method())
    {
    case ApiMethod::Customers: {
        auto customers = reply.get<Customers>();
        for (const auto& customer : customers)
        {
            mUi->cbCustomer->addItem(customer.name, customer.id);
        }
    }
    break;

    default:
        break;
    }
}

void ProjectDialog::validateForm()
{
    const auto& name  = mUi->leName->text();
    bool nameOk       = (not name.isEmpty()) and (name.size() > 1);
    bool customerOk   = not mUi->cbCustomer->currentText().isEmpty();
    bool timeBudgetOk = mUi->leTimeBudget->hasAcceptableInput();
    enableSave(nameOk and customerOk and timeBudgetOk);
}
