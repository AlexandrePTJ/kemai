#include "activitydialog.h"
#include "ui_activitydialog.h"

#include "datareader.h"

#include <QPushButton>

using namespace kemai::app;
using namespace kemai::client;

ActivityDialog::ActivityDialog(QWidget* parent) : QDialog(parent), mUi(new Ui::ActivityDialog)
{
    mUi->setupUi(this);
    enableSave(false);

//    connect(mUi->leName, &QLineEdit::textChanged, this, &ActivityDialog::validateForm);
//
//    const auto& countries = DataReader::countries();
//    for (auto it = countries.begin(); it != countries.end(); ++it)
//    {
//        mUi->cbCountry->addItem(it.value(), it.key());
//    }
//    const auto& currencies = DataReader::currencies();
//    for (auto it = currencies.begin(); it != currencies.end(); ++it)
//    {
//        mUi->cbCurrency->addItem(it.value(), it.key());
//    }
//    const auto& timezones = DataReader::timezones();
//    mUi->cbTimezone->addItems(timezones);
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
    return activity;
}

void ActivityDialog::enableSave(bool enable)
{
//    auto btn = mUi->buttonBox->button(QDialogButtonBox::Save);
//    if (btn)
//        btn->setEnabled(enable);
}

void ActivityDialog::validateForm()
{
//    bool nameOk = not mUi->leName->text().isEmpty();
//    enableSave(nameOk);
}
