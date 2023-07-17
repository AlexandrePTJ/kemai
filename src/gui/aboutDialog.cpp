#include "aboutDialog.h"
#include "ui_aboutDialog.h"

#include "kemaiConfig.h"

using namespace kemai;

AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent), mUi(std::make_unique<Ui::AboutDialog>())
{
    mUi->setupUi(this);
    mUi->versionLabel->setText(KEMAI_VERSION);
    mUi->licencesTextBrowser->setSource({"qrc:/misc/licences"});
}

AboutDialog::~AboutDialog() = default;
