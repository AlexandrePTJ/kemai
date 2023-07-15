#include "activityDialog.h"
#include "ui_activityDialog.h"

#include <QPushButton>
#include <QTimer>

using namespace kemai;

ActivityDialog::ActivityDialog(QWidget* parent) : QDialog(parent), mUi(std::make_unique<Ui::ActivityDialog>())
{
    mUi->setupUi(this);
    enableSave(false);

    connect(mUi->leName, &QLineEdit::textChanged, this, &ActivityDialog::validateForm);
    connect(mUi->leTimeBudget, &QLineEdit::textChanged, this, &ActivityDialog::validateForm);
}

ActivityDialog::~ActivityDialog() = default;

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
    auto button = mUi->buttonBox->button(QDialogButtonBox::Save);
    if (button != nullptr)
    {
        button->setEnabled(enable);
    }
}

void ActivityDialog::validateForm()
{
    const auto& name  = mUi->leName->text();
    bool nameOk       = (!name.isEmpty()) && (name.size() > 1);
    bool timeBudgetOk = mUi->leTimeBudget->hasAcceptableInput();
    enableSave(nameOk && timeBudgetOk);
}
