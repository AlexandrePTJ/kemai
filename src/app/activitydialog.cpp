#include "activitydialog.h"
#include "ui_activitydialog.h"

#include "helpers.h"

#include "client/kimairequestfactory.h"

#include <QPushButton>
#include <QTimer>

using namespace kemai::app;
using namespace kemai::client;

ActivityDialog::ActivityDialog(QWidget* parent) : QDialog(parent), mUi(new Ui::ActivityDialog)
{
    mUi->setupUi(this);
    enableSave(false);

    mClient = helpers::createClient();
    if (mClient)
    {
        connect(mClient.data(), &KimaiClient::replyReceived, this, &ActivityDialog::onClientReply);
        QTimer::singleShot(200, [=]() { mClient->sendRequest(KimaiRequestFactory::customers()); });
    }

    connect(mUi->cbCustomer, &QComboBox::currentTextChanged, this, &ActivityDialog::onCbCustomerTextChanged);
    connect(mUi->cbProject, &QComboBox::currentTextChanged, this, &ActivityDialog::validateForm);
    connect(mUi->leName, &QLineEdit::textChanged, this, &ActivityDialog::validateForm);
    connect(mUi->leTimeBudget, &QLineEdit::textChanged, this, &ActivityDialog::validateForm);
}

ActivityDialog::~ActivityDialog()
{
    delete mUi;
}

void ActivityDialog::setActivity(const Activity& activity)
{
    validateForm();
}

Activity ActivityDialog::activity() const
{
    Activity activity;
    activity.name       = mUi->leName->text();
    activity.budget     = mUi->sbBudget->value();
    activity.timeBudget = mUi->leTimeBudget->seconds();

    auto idCustomer = mUi->cbCustomer->currentData(Qt::UserRole).toInt();
    auto idProject  = mUi->cbProject->currentData(Qt::UserRole).toInt();

    if (idCustomer >= 0)
    {
        Project project;
        project.customer.id = idCustomer;
        project.id          = idProject;

        activity.project = project;
    }
    return activity;
}

void ActivityDialog::enableSave(bool enable)
{
    auto btn = mUi->buttonBox->button(QDialogButtonBox::Save);
    if (btn)
        btn->setEnabled(enable);
}

void ActivityDialog::onClientReply(const KimaiReply& reply)
{

    switch (reply.method())
    {
    case ApiMethod::Customers: {
        auto customers = reply.get<Customers>();
        mUi->cbCustomer->addItem("", -1);
        for (const auto& customer : customers)
        {
            mUi->cbCustomer->addItem(customer.name, customer.id);
        }
    }
    break;

    case ApiMethod::Projects: {
        mUi->cbProject->addItem("", -1);

        for (const auto& project : reply.get<Projects>())
            mUi->cbProject->addItem(project.name, project.id);
    }
    break;

    default:
        break;
    }
}

void ActivityDialog::onCbCustomerTextChanged(const QString& text)
{
    mUi->cbProject->clear();

    if (not text.isEmpty() and mClient)
    {
        auto customerId = mUi->cbCustomer->currentData().toInt();
        mClient->sendRequest(KimaiRequestFactory::projects(customerId));
    }

    validateForm();
}

void ActivityDialog::validateForm()
{
    const auto& name = mUi->leName->text();
    bool nameOk      = (not name.isEmpty()) and (name.size() > 1);
    bool projectOk   = mUi->cbCustomer->currentText().isEmpty() or
                     (not mUi->cbCustomer->currentText().isEmpty() and not mUi->cbProject->currentText().isEmpty());
    bool timeBudgetOk = mUi->leTimeBudget->hasAcceptableInput();
    enableSave(nameOk and projectOk and timeBudgetOk);
}
