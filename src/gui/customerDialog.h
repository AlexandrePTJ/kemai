#pragma once

#include <memory>

#include <QDialog>

#include "client/kimaiAPI.h"

namespace Ui {
class CustomerDialog;
}

namespace kemai {

class CustomerDialog : public QDialog
{
    Q_OBJECT

public:
    CustomerDialog(QWidget* parent = nullptr);
    ~CustomerDialog() override;

    Customer customer() const;

private:
    void enableSave(bool enable);
    void validateForm();

    std::unique_ptr<Ui::CustomerDialog> mUi;
};

} // namespace kemai
