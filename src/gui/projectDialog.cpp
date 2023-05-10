#include "projectDialog.h"
#include "ui_projectDialog.h"

#include <QPushButton>
#include <QTimer>

using namespace kemai;

ProjectDialog::ProjectDialog(QWidget* parent) : QDialog(parent), mUi(new Ui::ProjectDialog)
{
    mUi->setupUi(this);
    enableSave(false);

    connect(mUi->leName, &QLineEdit::textChanged, this, &ProjectDialog::validateForm);
    connect(mUi->leTimeBudget, &QLineEdit::textChanged, this, &ProjectDialog::validateForm);
}

ProjectDialog::~ProjectDialog()
{
    delete mUi;
}

Project ProjectDialog::project() const
{
    Project project;
    project.name       = mUi->leName->text();
    project.budget     = mUi->sbBudget->value();
    project.timeBudget = mUi->leTimeBudget->seconds();
    return project;
}

void ProjectDialog::enableSave(bool enable)
{
    if (auto btn = mUi->buttonBox->button(QDialogButtonBox::Save))
    {
        btn->setEnabled(enable);
    }
}

void ProjectDialog::validateForm()
{
    const auto& name  = mUi->leName->text();
    bool nameOk       = (!name.isEmpty()) && (name.size() > 1);
    bool timeBudgetOk = mUi->leTimeBudget->hasAcceptableInput();
    enableSave(nameOk && timeBudgetOk);
}
