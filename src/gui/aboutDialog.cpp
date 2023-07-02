#include "aboutDialog.h"
#include "ui_aboutDialog.h"

#include "kemaiConfig.h"

using namespace kemai;

AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent), mUi(new Ui::AboutDialog)
{
    mUi->setupUi(this);
    mUi->versionLabel->setText(KEMAI_VERSION);
    mUi->licencesTextBrowser->setSource({"qrc:/misc/licences"});
}

AboutDialog::~AboutDialog()
{
    delete mUi;
}
