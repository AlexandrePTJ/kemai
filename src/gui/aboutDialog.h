#pragma once

#include <QDialog>

namespace Ui {
class AboutDialog;
}

namespace kemai {

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    AboutDialog(QWidget* parent = nullptr);
    ~AboutDialog() override;

private:
    Ui::AboutDialog* mUi;
};

} // namespace kemai
