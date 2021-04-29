#include "activitydialog.h"
#include "ui_activitydialog.h"

#include <QPushButton>
#include <QTimer>

using namespace kemai::app;
using namespace kemai::client;

ActivityDialog::ActivityDialog(QWidget* parent) : QDialog(parent), mUi(new Ui::ActivityDialog)
{
    mUi->setupUi(this);
    enableSave(false);

    connect(mUi->leName, &QLineEdit::textChanged, this, &ActivityDialog::validateForm);
    connect(mUi->leTimeBudget, &QLineEdit::textChanged, this, &ActivityDialog::validateForm);
}

ActivityDialog::~ActivityDialog()
{
    delete mUi;
}

Activity ActivityDialog::activity() const
{
    Activity activity;
    activity.name       = mUi->leName->text();
    activity.budget     = mUi->sbBudget->value();
    activity.timeBudget = mUi->leTimeBudget->seconds();
    return activity;
}

void ActivityDialog::enableSave(bool enable)
{
    auto btn = mUi->buttonBox->button(QDialogButtonBox::Save);
    if (btn)
        btn->setEnabled(enable);
}

void ActivityDialog::validateForm()
{
    const auto& name  = mUi->leName->text();
    bool nameOk       = (not name.isEmpty()) and (name.size() > 1);
    bool timeBudgetOk = mUi->leTimeBudget->hasAcceptableInput();
    enableSave(nameOk and timeBudgetOk);
}
