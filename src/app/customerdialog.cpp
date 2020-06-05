#include "customerdialog.h"
#include "ui_customerdialog.h"

#include "datareader.h"

#include <QPushButton>
#include <QTimeZone>

using namespace kemai::app;
using namespace kemai::client;

CustomerDialog::CustomerDialog(QWidget* parent) : QDialog(parent), mUi(new Ui::CustomerDialog)
{
    mUi->setupUi(this);
    enableSave(false);

    connect(mUi->leName, &QLineEdit::textChanged, this, &CustomerDialog::validateForm);
    connect(mUi->leTimeBudget, &QLineEdit::textChanged, this, &CustomerDialog::validateForm);

    const auto& countries = DataReader::countries();
    for (auto it = countries.begin(); it != countries.end(); ++it)
    {
        mUi->cbCountry->addItem(it.value(), it.key());
    }
    const auto& currencies = DataReader::currencies();
    for (auto it = currencies.begin(); it != currencies.end(); ++it)
    {
        mUi->cbCurrency->addItem(it.value(), it.key());
    }

    for (const auto& tz : QTimeZone::availableTimeZoneIds())
        mUi->cbTimezone->addItem(tz);
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
    customer.name        = mUi->leName->text();
    customer.countryKey  = mUi->cbCountry->currentData(Qt::UserRole).toString();
    customer.currencyKey = mUi->cbCurrency->currentData(Qt::UserRole).toString();
    customer.timezone    = mUi->cbTimezone->currentText();
    customer.budget      = mUi->sbBudget->value();
    customer.timeBudget  = mUi->leTimeBudget->seconds();

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
    const auto& name  = mUi->leName->text();
    bool nameOk       = (not name.isEmpty()) and (name.size() > 1);
    bool timeBudgetOk = mUi->leTimeBudget->hasAcceptableInput();
    enableSave(nameOk and timeBudgetOk);
}
