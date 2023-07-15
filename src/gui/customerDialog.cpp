#include "customerDialog.h"
#include "ui_customerDialog.h"

#include "misc/dataReader.h"

#include <QPushButton>
#include <QTimeZone>

using namespace kemai;

CustomerDialog::CustomerDialog(QWidget* parent) : QDialog(parent), mUi(std::make_unique<Ui::CustomerDialog>())
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
    {
        mUi->cbTimezone->addItem(tz);
    }
}

CustomerDialog::~CustomerDialog() = default;

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
    if (auto btn = mUi->buttonBox->button(QDialogButtonBox::Save))
    {
        btn->setEnabled(enable);
    }
}

void CustomerDialog::validateForm()
{
    const auto& name  = mUi->leName->text();
    bool nameOk       = (!name.isEmpty()) && (name.size() > 1);
    bool timeBudgetOk = mUi->leTimeBudget->hasAcceptableInput();
    enableSave(nameOk && timeBudgetOk);
}
