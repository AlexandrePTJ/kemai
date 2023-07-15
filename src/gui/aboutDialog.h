#pragma once

#include <memory>

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
    std::unique_ptr<Ui::AboutDialog> mUi;
};

} // namespace kemai
