#include "projectdialog.h"
#include "ui_projectdialog.h"

#include "client/kimairequestfactory.h"

#include <QPushButton>
#include <QTimer>

using namespace kemai::app;
using namespace kemai::client;

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

void ProjectDialog::setProject(const Project& project)
{
    validateForm();
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
    auto btn = mUi->buttonBox->button(QDialogButtonBox::Save);
    if (btn)
        btn->setEnabled(enable);
}

void ProjectDialog::validateForm()
{
    const auto& name  = mUi->leName->text();
    bool nameOk       = (not name.isEmpty()) and (name.size() > 1);
    bool timeBudgetOk = mUi->leTimeBudget->hasAcceptableInput();
    enableSave(nameOk and timeBudgetOk);
}
